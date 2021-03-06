/*
    ELF file format parsing/loading

    Part of the as-yet-unnamed MC68010 operating system


    Stuart Wallace, August 2015.
*/

#include <kernel/include/byteorder.h>
#include <kernel/include/elf.h>
#include <kernel/include/error.h>
#include <klibc/include/errno.h>
#include <klibc/include/stdio.h>
#include <klibc/include/string.h>
#include <klibc/include/strings.h>


/*
    elf_load_exe() - load an executable ELF image.  buf should point to an in-memory copy of the
    executable file; len should specify buf's length.
*/
s32 elf_load_exe(const void * const buf, ku32 len, exe_img_t **img)
{
    const Elf32_Ehdr *ehdr;
    const Elf32_Shdr *shdr, *sh;
    exe_img_t *image;
    s8 *strtab;
    u8 *imgbuf;
    u32 vaddr_start, vaddr_end, size;
    u16 nshdr;


    /* ELF header sits at the start of the executable image */
    ehdr = (Elf32_Ehdr *) buf;

    /* Verify ELF magic numbers */
    if((ehdr->e_ident[EI_MAG0] != ELFMAG0) || (ehdr->e_ident[EI_MAG1] != ELFMAG1) ||
        (ehdr->e_ident[EI_MAG2] != ELFMAG2) || (ehdr->e_ident[EI_MAG3] != ELFMAG3))
    {
        return -EEXEBADHDR;
    }

    /* Verify that this is a 32-bit ELF file */
    if(ehdr->e_ident[EI_CLASS] != ELFCLASS32)
        return -EEXEBADARCH;

    /* Get the endianness of the file.  We only support big-endian files. */
    if(ehdr->e_ident[EI_DATA] != ELFDATA2MSB)
        return -EEXEENDIAN;

    /* Verify that this is an executable image (vs. an object file, etc.) */
    if(BE2N16(ehdr->e_type) != ET_EXEC)
        return -ENOTEXE;

    /* Verify that this file matches our architecture */
    /* FIXME: architecture-specific */
    if((BE2N16(ehdr->e_machine) != EM_68K) ||
        ((BE2N32(ehdr->e_flags) & EF_M68K_ARCH_MASK) != EF_M68K_M68000))
    {
        return -EEXEBADARCH;
    }

    if(!ehdr->e_phoff)
        return -EEXENOSECTION;      /* Program header table missing */

    /* Parse section headers */
    shdr = (Elf32_Shdr *) ((u8 *) buf + BE2N32(ehdr->e_shoff));
    nshdr = BE2N16(ehdr->e_shnum);

    /* Check that the section headers don't run past the end of the file */
    if((u8 *) &shdr[nshdr] > ((u8 *) buf + len))
        return -EEXEBADSECTION;

    /* Search the section headers for the string table */
    for(strtab = NULL, sh = shdr; sh < &shdr[nshdr]; ++sh)
        if((BE2N32(sh->sh_type) == SHT_STRTAB) && sh->sh_size)
            strtab = (s8 *) buf + BE2N32(sh->sh_offset);        /* Found the string table */

    if(strtab == NULL)
        return -EEXENOSECTION;      /* String table missing */

    /* Search the section headers for program sections */
    vaddr_start = 0xffffffff;
    vaddr_end = 0;

    /* Pass 1: calculate extents of the memory region required for the program */
    for(sh = shdr; sh < &shdr[nshdr]; ++sh)
    {
        ks8 * const name = strtab + BE2N32(sh->sh_name);
        if(((BE2N32(sh->sh_type) == SHT_PROGBITS) && elf_is_relevant_progbits_section(name))
            || ((BE2N32(sh->sh_type) == SHT_NOBITS) && elf_is_relevant_nobits_section(name)))
        {
            /* This section will be loaded */
            if(BE2N32(sh->sh_addr) < vaddr_start)
                vaddr_start = BE2N32(sh->sh_addr);

            if((BE2N32(sh->sh_addr) + BE2N32(sh->sh_size)) > vaddr_end)
                vaddr_end = BE2N32(sh->sh_addr) + BE2N32(sh->sh_size);
        }
    }

    size = vaddr_end - vaddr_start;

    imgbuf = (u8 *) umalloc(size);
    if(!imgbuf)
        return -ENOMEM;

    image = kmalloc(sizeof(exe_img_t));
    if(!image)
    {
        ufree(imgbuf);
        return -ENOMEM;
    }

    /* Pass 2: copy segments into buffer; initialise as required */
    for(sh = shdr; sh < &shdr[nshdr]; ++sh)
    {
        ks8 * const name = strtab + BE2N32(sh->sh_name);
        u8 * const sect_start_img = imgbuf + (BE2N32(sh->sh_addr) - vaddr_start),
           * const sect_start_buf = (u8 *) buf + BE2N32(sh->sh_offset);

        if((BE2N32(sh->sh_type) == SHT_PROGBITS) && elf_is_relevant_progbits_section(name))
        {
            /* Copy the section to the buffer */
            memcpy(sect_start_img, sect_start_buf, BE2N32(sh->sh_size));

            if(!strcmp(name, ".got"))
            {
                /* .got section: adjust offset addresses appropriately */
                u32 *got_entry = (u32 *) sect_start_img,
                    *got_end = (u32 *) (sect_start_img + BE2N32(sh->sh_size));

                for(; got_entry < got_end; ++got_entry)
                    if(*got_entry)
                        *got_entry = BE2N32(BE2N32(*got_entry) - vaddr_start + (u32) imgbuf);
            }
        }
        else if((BE2N32(sh->sh_type) == SHT_NOBITS) && elf_is_relevant_nobits_section(name))
            bzero(sect_start_img, BE2N32(sh->sh_size)); /* Zero-init NOBITS sections (eg. .bss) */
    }


    image->start = imgbuf;
    image->len = size;
    image->entry_point = (proc_entry_fn_t) (imgbuf + (BE2N32(ehdr->e_entry) - vaddr_start));

    *img = image;

    return SUCCESS;
}


/*
    elf_is_relevant_progbits_section() - given the name of a section, return true if it is a
    populated program section that needs to be processed; return false otherwise.
*/
u32 elf_is_relevant_progbits_section(ks8 * name)
{
    return !strcmp(name, ".text") || !strcmp(name, ".rodata")
            || !strcmp(name, ".got") || !strcmp(name, ".data");
}


/*
    elf_is_relevant_nobits_section() - given the name of a section, return true if it is a section
    taking up no space in the ELF file, but still relevant; return false otherwise.
*/
u32 elf_is_relevant_nobits_section(ks8 * name)
{
    return !strcmp(name, ".bss");
}

