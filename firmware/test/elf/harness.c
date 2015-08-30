/*
	Test functions used by the ELF loader test script

	Part of the as-yet-unnamed MC68010 operating system


	Stuart Wallace, August 2015.
*/

#include "harness.h"

u16 bswap_16(u16 val)
{
	return ((val >> 8) & 0xff) | (val << 8);
}


u32 bswap_32(u32 val)
{
	return (val << 24) | ((val & 0xff00) << 8) | ((val & 0xff0000) >> 8) | ((val >> 24) & 0xff);
}


void *umalloc(u32 size)
{
	return malloc(size);
}


void elf_dump_hdr(const Elf32_Ehdr * const h)
{
	printf("ELF32 header\n"
		   "e_type      = %u\n"
		   "e_machine   = %u\n"
		   "e_version   = %u\n"
		   "e_entry     = 0x%08x\n"
		   "e_phoff     = %u\n"
		   "e_shoff     = %u\n"
		   "e_flags     = 0x%08x\n"
		   "e_ehsize    = %u\n"
		   "e_phentsize = %u\n"
		   "e_phnum     = %u\n"
		   "e_shentsize = %u\n"
		   "e_shnum     = %u\n"
		   "e_shstrndx  = %u\n",
		   BE2N16(h->e_type), BE2N16(h->e_machine), BE2N32(h->e_version), BE2N32(h->e_entry),
		   BE2N32(h->e_phoff), BE2N32(h->e_shoff), BE2N32(h->e_flags), BE2N16(h->e_ehsize),
		   BE2N16(h->e_phentsize), BE2N16(h->e_phnum), BE2N16(h->e_shentsize), BE2N16(h->e_shnum),
		   BE2N16(h->e_shstrndx));
}


void elf_dump_phdr(const Elf32_Phdr * const h)
{
	printf("ELF32 program header\n"
		   "p_type   = %u\n"
		   "p_offset = %u\n"
		   "p_vaddr  = 0x%08x\n"
		   "p_paddr  = 0x%08x\n"
		   "p_filesz = %u\n"
		   "p_memsz  = %u\n"
		   "p_flags  = 0x%08x\n"
		   "p_align  = %u\n",
		   BE2N32(h->p_type), BE2N32(h->p_offset), BE2N32(h->p_vaddr), BE2N32(h->p_paddr),
		   BE2N32(h->p_filesz), BE2N32(h->p_memsz), BE2N32(h->p_flags), BE2N32(h->p_align));
}


void elf_dump_shdr(const Elf32_Shdr * const h, ks8 *stab)
{
	printf("ELF32 section header\n"
		   "sh_name      = %u (%s)\n"
		   "sh_type      = %u\n"
		   "sh_flags     = 0x%08x\n"
		   "sh_addr	     = 0x%08x\n"
		   "sh_offset    = %u\n"
		   "sh_size	     = %u\n"
		   "sh_link	     = %u\n"
		   "sh_info      = %u\n"
		   "sh_addralign = %u\n"
		   "sh_entsize   = %u\n",
		   BE2N32(h->sh_name), stab + BE2N32(h->sh_name), BE2N32(h->sh_type), BE2N32(h->sh_flags),
		   BE2N32(h->sh_addr), BE2N32(h->sh_offset), BE2N32(h->sh_size), BE2N32(h->sh_link),
		   BE2N32(h->sh_info), BE2N32(h->sh_addralign), BE2N32(h->sh_entsize));
}


void old_program_header_inspection_code()
{
	/*
		[2015-08-30] This code was used to inspect program header segments.  I don't think this is
		necessary in order to load an executable with minimal (GOT-only) relocation, so I removed
		it.  I have left it here in case it proves useful in future.
	*/
#if 0
	const Elf32_Phdr *phdr, *ph;
	u16 nphdr, pass;

	/* Parse program header and load sections */
	phdr = (Elf32_Phdr *) ((u8 *) buf + BE2N32(ehdr->e_phoff));
	nphdr = BE2N16(ehdr->e_phnum);

	/* Check that the program header sections don't run past the end of the file */
	if((u8 *) &phdr[nphdr] > ((u8 *) buf + len))
	{
		printf("Invalid program header table offset %08x\n", (u32) ((u8 *) phdr - (u8 *) buf));
		return EINVAL;
	}

	/* Iterate over segments; find loadable segs; allocate memory; load the data */
	vaddr_start = 0xffffffff;
	vaddr_end = 0;
	for(pass = 0; pass < 2; ++pass)
	{
		for(ph = phdr; ph < &phdr[nphdr]; ++ph)
		{
			if(BE2N32(phdr->p_type) != PT_LOAD)
				continue;	/* Only interested in loadable regions */

			if(pass == 0)
			{
				/* Pass 0: determine the virtual address range required by the executable */
				elf_dump_phdr(ph);

				if(BE2N32(ph->p_vaddr) < vaddr_start)
					vaddr_start = BE2N32(ph->p_vaddr);

				if((BE2N32(ph->p_vaddr) + BE2N32(ph->p_memsz)) > vaddr_end)
					vaddr_end = (BE2N32(ph->p_vaddr) + BE2N32(ph->p_memsz));
			}
			else
			{
				/* Pass 2: copy segments into memory */
				
			}
		}

		if(pass == 0)
		{
			size = vaddr_end - vaddr_start;
			printf("Memory region start=0x%08x end=0x%08x, size=%u\n",
					vaddr_start, vaddr_end, size);

			*membuf = umalloc(size);
			if(!*membuf)
			{
				printf("Failed to allocate %u bytes\n", size);
				return ENOMEM;
			}
		}
	}
#endif
}

