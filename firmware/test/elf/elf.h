#ifndef KERNEL_ELF_H
#define KERNEL_ELF_H
/*
	ELF file format parsing/loading

	Part of the as-yet-unnamed MC68010 operating system


	Stuart Wallace, August 2015.
*/


#include "include/types.h"


/* Object file types (Elf32_Ehdr.e_type) */
#define ET_NONE				(0)			/* No file type				*/
#define ET_REL				(1)			/* Relocatable file			*/
#define ET_EXEC				(2)			/* Executable file			*/
#define ET_DYN				(3)			/* Shared object file		*/
#define ET_CORE				(4)			/* Core file 				*/
#define ET_LOPROC			(0xff00)	/* Processor-specific		*/
#define ET_HIPROC			(0xffff)	/* Processor-specific		*/

/* Machine types (Elf32_Ehdr.e_machine) */
#define EM_NONE				(0)			/* No machine				*/
#define EM_M32				(1)			/* AT&T WE 32100			*/
#define EM_SPARC			(2)			/* SPARC					*/
#define EM_386				(3)			/* Intel 80386 				*/
#define EM_68K				(4)			/* Motorola 68000			*/
#define EM_88K				(5)			/* Motorola 88000			*/
#define EM_860				(7)			/* Intel 80860				*/
#define EM_MIPS				(8)			/* MIPS RS3000				*/

/* Object file versions (Elf32_Ehdr.e_version) */
#define EV_NONE				(0)			/* No version				*/
#define EV_CURRENT			(1)			/* Current version			*/

/* Size of Elf32.Ehdr.e_ident[] char array */
#define EI_NIDENT			(16)

/* Elf32.Ehdr.e_ident[] indices */
#define EI_MAG0				(0)			/* File identification		*/
#define EI_MAG1				(1)			/* File identification		*/
#define EI_MAG2				(2)			/* File identification		*/
#define EI_MAG3				(3)			/* File identification		*/
#define EI_CLASS			(4)			/* File class				*/
#define EI_DATA				(5)			/* Data encoding			*/
#define EI_VERSION			(6)			/* File version				*/
#define EI_PAD				(7)			/* Start of padding bytes	*/

/* File identification magic numbers (Elf32_Ehdr.e_ident[0..3]) */
#define ELFMAG0				(0x7f)
#define ELFMAG1				('E')
#define ELFMAG2				('L')
#define ELFMAG3				('F')

/* File classes (Elf32_Ehdr.e_ident[EI_CLASS]) */
#define ELFCLASSNONE		(0)			/* Invalid class			*/
#define ELFCLASS32			(1)			/* 32-bit objects			*/
#define ELFCLASS64			(2)			/* 64-bit objects			*/

/* Encoding of processor-specific data (Elf32_Ehdr.e_ident[EI_DATA]) */
#define ELFDATANONE			(0)			/* Invalid data encoding	*/
#define ELFDATA2LSB			(1)			/* Little-endian encoding	*/
#define ELFDATA2MSB			(2)			/* Big-endian encoding		*/

/* ELF header structure */
typedef struct
{
	u8		e_ident[EI_NIDENT];	/* Identification bytes 										*/
	u16		e_type;				/* Object file type 											*/
	u16		e_machine;			/* Machine architecture 										*/
	u32		e_version;			/* Object file version 											*/
	u32		e_entry;			/* Entry point address 											*/
	u32		e_phoff;			/* Program header table file offset 							*/
	u32		e_shoff;			/* Section header table file offset 							*/
	u32		e_flags;			/* Processor-specific flags 									*/
	u16		e_ehsize;			/* ELF header size 												*/
	u16		e_phentsize;		/* Size of a program header table entry 						*/
	u16		e_phnum;			/* Number of program header table entries						*/
	u16		e_shentsize;		/* Size of a section header table entry							*/
	u16		e_shnum;			/* Number of section header table entries 						*/
	u16		e_shstrndx;			/* Index of section-name string table in section header table	*/
} Elf32_Ehdr;

/* Special section indices */
#define SHN_UNDEF			(0)				/* Undefined/missing section					*/
#define SHN_LORESERVE		(0xff00)		/* Lower bound of reserved indices				*/
#define SHN_LOPROC			(0xff00)		/* Processor-specific semantics - lower bound	*/
#define SHN_HIPROC			(0xff1f)		/* Processor-specific semantics - upper bound	*/
#define SHN_ABS				(0xfff1)		/* Absolute values (immune to relocation)		*/
#define SHN_COMMON			(0xfff2)		/* Common symbols								*/
#define SHN HIRESERVE		(0xffff)		/* Upper bound of reserved indices				*/

/* Section header types (Elf32_Shdr.sh_type) */
#define SHT_NULL			(0)				/* Inactive header; no associated section		*/
#define SHT_PROGBITS		(1)				/* Program-specific information					*/
#define SHT_SYMTAB			(2)				/* Symbols for link editing						*/
#define SHT_STRTAB			(3)				/* String table									*/
#define SHT_RELA			(4)				/* Relocation entries with explicit addends		*/
#define SHT_HASH			(5)				/* Hash table									*/
#define SHT_DYNAMIC			(6)				/* Information for dynamic linking				*/
#define SHT_NOTE			(7)				/* Notes										*/
#define SHT_NOBITS			(8)				/* Like SHT_PROGBITS, but occupies no space		*/
#define SHT_REL				(9)				/* Relocation entries without explicit addends	*/
#define SHT_SHLIB			(10)			/* Reserved; unspecified semantics				*/
#define SHT_DYNSYM			(11)			/* Symbols for dynamic linking					*/
#define SHT_LOPROC			(0x70000000)	/* Processor-specific semantics - lower bound	*/
#define SHT_HIPROC			(0x7fffffff)	/* Processor-specific semantics - upper bound	*/
#define SHT_LOUSER			(0x80000000)	/* App-specific indices - lower bound			*/
#define SHT_HIUSER			(0xffffffff)	/* App-specific indices - upper bound			*/

/* Section attribute flags (Elf32_Shdr.sh_flags) */
#define SHF_WRITE			(0x1)			/* Data should be writable during execution		*/
#define SHF_ALLOC			(0x2)			/* Section occupies memory during execution		*/
#define SHF_EXECINSTR		(0x4)			/* Section contains executable machine instrs	*/
#define SHF_MASKPROC		(0xf0000000)	/* Mask for processor-specific semantics		*/

/* Section header structure */
typedef struct
{
	u32		sh_name;			/* Name of section (index into section header string table)		*/
	u32		sh_type;			/* Section type													*/
	u32		sh_flags;			/* Miscellaneous attribute flags								*/
	u32		sh_addr;			/* Memory address at which first byte of section should reside	*/
	u32		sh_offset;			/* Offset from beginning of file to first byte in section		*/
	u32		sh_size;			/* Size of section												*/
	u32		sh_link;			/* Section header table index link								*/
	u32		sh_info;			/* Extra information, section-type specific						*/
	u32		sh_addralign;		/* Address alignment required for section						*/
	u32		sh_entsize;			/* For sections containing fixed-size entries: size of an entry	*/
} Elf32_Shdr;

/* Symbol table "undefined symbol" index */
#define STN_UNDEF			(0)				/* Undefined symbol */

/* Extractor macros for symbol type and binding attributes (Elf32_Sym.st_info) */
#define ELF32_ST_BIND(i)	((i) >> 4)
#define ELF32_ST_TYPE(i)	((i) & 0xf)
#define ELF32_ST_INFO(b, t)	(((b) << 4) + ((t) & 0xf))

/* Symbol binding types (Elf32_Sym.st_info) */
#define STB_LOCAL			(0)		/* Local symbol - not visible outside the file			*/
#define STB_GLOBAL			(1)		/* Global symbol - visible to all files being combined	*/
#define STB_WEAK			(2)		/* Global symbol with lower precedence					*/
#define STB_LOPROC			(13)	/* Processor-specific semantics - lower bound			*/
#define STB_HIPROC			(15)	/* Processor-specific semantics - upper bound			*/

/* Symbol types (Elf32_Sym.st_info) */
#define STT_NOTYPE			(0)		/* Unspecified type										*/
#define STT_OBJECT			(1)		/* Symbol is a data object (variable, array, ...)		*/
#define STT_FUNC			(2)		/* Symbol is a function or other executable code		*/
#define STT_SECTION			(3)		/* Symbol is a section									*/
#define STT_FILE			(4)		/* Symbol represents a file (e.g. a source file)		*/
#define STT_LOPROC			(13)	/* Processor-specific semantics - lower bound			*/
#define STT_HIPROC			(15)	/* Processor-specific semantics - upper bound			*/

/* Symbol table entry */
typedef struct
{
	u32		st_name;			/* Index of symbol's name in string table		*/
	u32		st_value;			/* Value of symbol								*/
	u32		st_size;			/* Size of symbol, if any						*/
	u8		st_info;			/* Symbol type and binding attributes			*/
	u8		st_other;			/* (no defined meaning)							*/
	u16		st_shndx;			/* Index of section in which symbol is defined	*/
} __attribute__((packed)) Elf32_Sym;

/* Relocation index/type extractor macros (Elf32_Rel*.r_info) */
#define ELF32_R_SYM(i)		((i) >> 8)
#define ELF32_R_TYPE(i)		((u8) (i))
#define ELF32_R_INFO(s, t)	(((s) << 8) + (u8) (t))

/* Relocation types */
#define R_386_NONE			(0)		/* No relocation									*/
#define R_386_32			(1)		/* 													*/
#define R_386_PC32			(2)		/* 													*/
#define R_386_GOT32			(3)		/* Offset of symbol's entry in GOT from GOT base	*/
#define R_386_PLT32			(4)		/* Address of symbol's PLT entry					*/
#define R_386_COPY			(5)		/* Copy symbol's data to specified offset			*/
#define R_386_GLOB_DAT		(6)		/* Set GOT entry to address of the symbol			*/
#define R_386_JMP_SLOT		(7)		/* Jump table entry in PLT							*/
#define R_386_RELATIVE		(8)		/* Relative address									*/
#define R_386_GOTOFF		(9)		/* 													*/
#define R_386_GOTPC			(10)	/* 													*/

/* Relocation entry */
typedef struct
{
	u32		r_offset;			/* Location at which to apply the relocation action		*/
	u32		r_info;				/* Index of symbol table entry and type of relocation	*/
} Elf32_Rel;

typedef struct
{
	u32		r_offset;			/* Location at which to apply the relocation action		*/
	u32		r_info;				/* Index of symbol table entry and type of relocation	*/
	s32		r_addend;			/* Constant addend used to compute the relocation value	*/
} Elf32_Rela;

#endif

