/*
	ELF file format parsing/loading

	Part of the as-yet-unnamed MC68010 operating system


	Stuart Wallace, August 2015.

	TODO: transparent endianness support
*/


/* Define these in order to avoid picking up the defs in the ayumos code FIXME remove*/
#define HAVE_SIZE_T	1
#define HAVE_PID_T	1

#define EINVAL 22			// FIXME remove

#include "elf.h"
#include <stdio.h>			// FIXME


void elf_dump_hdr(const Elf32_Ehdr * const ehdr)
{
	printf("e_type      = %u\n"
		   "e_machine   = %u\n"
		   "e_version   = %u\n"
		   "e_entry     = 0x%08x\n"
		   "e_phoff     = %u\n"
		   "e_shoff     = %u\n"
		   "e_flags     = %u\n"
		   "e_ehsize    = %u\n"
		   "e_phentsize = %u\n"
		   "e_phnum     = %u\n"
		   "e_shentsize = %u\n"
		   "e_shnum     = %u\n"
		   "e_shstrndx  = %u\n",
		   ehdr->e_type, ehdr->e_machine, ehdr->e_version, ehdr->e_entry, ehdr->e_phoff,
		   ehdr->e_shoff, ehdr->e_flags, ehdr->e_ehsize, ehdr->e_phentsize, ehdr->e_phnum,
		   ehdr->e_shentsize, ehdr->e_shnum, ehdr->e_shstrndx);
}


/*
	elf_load_exe() - load an executable ELF image.  buf should point to an in-memory copy of the
	executable file.
*/
s32 elf_load_exe(const void * const buf, ku32 len)
{
	const Elf32_Ehdr *ehdr;

	ehdr = (Elf32_Ehdr *) buf;

	puts("Checking ELF magic numbers...");

	/* Verify ELF magic numbers */
	if((ehdr->e_ident[EI_MAG0] != ELFMAG0) || (ehdr->e_ident[EI_MAG1] != ELFMAG1) ||
		(ehdr->e_ident[EI_MAG2] != ELFMAG2) || (ehdr->e_ident[EI_MAG3] != ELFMAG3))
	{
		printf("Bad ELF header: %02x %02x %02x %02x\n",
				ehdr->e_ident[EI_MAG0], ehdr->e_ident[EI_MAG1],
				ehdr->e_ident[EI_MAG2], ehdr->e_ident[EI_MAG3]);

		return EINVAL;
	}

	/* Verify that this is a 32-bit ELF file */
	if(ehdr->e_ident[EI_CLASS] != ELFCLASS32)
	{
		puts("This is not a 32-bit ELF file");

		return EINVAL;
	}

	/* Get the endianness of the file */
	if(ehdr->e_ident[EI_DATA] == ELFDATA2LSB)
	{
		puts("This is a little-endian file");
	}
	else if(ehdr->e_ident[EI_DATA] == ELFDATA2MSB)
	{
		puts("This is a big-endian file");
	}
	else
	{
		printf("Invalid endianness field value %02x\n", ehdr->e_ident[EI_DATA]);
		return EINVAL;
	}

	/* Verify that this is an executable image (vs. an object file, etc.) */
	if(ehdr->e_type != ET_EXEC)
	{
		printf("This is not an executable file (e_type=%d)\n", ehdr->e_type);

		return EINVAL;
	}

	puts("Checking program type...");

	elf_dump_hdr(ehdr);

	printf("Program header offset=%u size=%u count=%u\n", ehdr->e_phoff, ehdr->e_phentsize, ehdr->e_phnum);

	if(ehdr->e_phoff)
	{
		const Elf32_Phdr * const phdr = (Elf32_Phdr *) ((u8 *) buf + ehdr->e_phoff);

		if(((u8 *) phdr - (u8 *) buf) > (len - sizeof(Elf32_Phdr)))
		{
			printf("Invalid program header table offset %08x\n", (u32) ((u8 *) phdr - (u8 *) buf));

			return EINVAL;
		}

		printf("Program header table is at offset %08x\n", (u32) ((u8 *) phdr - (u8 *) buf));

		if(phdr->p_type != PT_LOAD)
		{
			printf("Bad program type: %08x\n", phdr->p_type);

			return EINVAL;
		}
	}
	else
	{
		puts("Image has no program header table");
	}
	

	return SUCCESS;
}

