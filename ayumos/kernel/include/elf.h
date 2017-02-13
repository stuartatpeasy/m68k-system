#ifndef KERNEL_INCLUDE_ELF_H
#define KERNEL_INCLUDE_ELF_H
/*
    ELF file format parsing/loading

    Part of the as-yet-unnamed MC68010 operating system


    Stuart Wallace, August 2015.
*/


#include <kernel/include/defs.h>
#include <kernel/include/types.h>


/* Object file types (Elf32_Ehdr.e_type) */
typedef enum
{
    ET_NONE             = 0,            /* No file type             */
    ET_REL              = 1,            /* Relocatable file         */
    ET_EXEC             = 2,            /* Executable file          */
    ET_DYN              = 3,            /* Shared object file       */
    ET_CORE             = 4,            /* Core file                */
    ET_LOPROC           = 0xff00,       /* Processor-specific       */
    ET_HIPROC           = 0xffff        /* Processor-specific       */
} Elf_FileType;

/* Machine types (Elf32_Ehdr.e_machine) */
typedef enum
{
    EM_NONE             = 0,        /* No machine                                           */
    EM_M32              = 1,        /* AT&T WE 32100                                        */
    EM_SPARC            = 2,        /* SPARC                                                */
    EM_386              = 3,        /* Intel 80386                                          */
    EM_68K              = 4,        /* Motorola 68000                                       */
    EM_88K              = 5,        /* Motorola 88000                                       */
    EM_860              = 7,        /* Intel 80860                                          */
    EM_MIPS             = 8,        /* MIPS RS3000                                          */
    EM_S370             = 9,        /* IBM System/370 Processor                             */
    EM_MIPS_RS3_LE      = 10,       /* MIPS RS3000 Little-endian                            */
    EM_PARISC           = 15,       /* Hewlett-Packard PA-RISC                              */
    EM_VPP500           = 17,       /* Fujitsu VPP500                                       */
    EM_SPARC32PLUS      = 18,       /* Enhanced instruction set SPARC                       */
    EM_960              = 19,       /* Intel 80960                                          */
    EM_PPC              = 20,       /* PowerPC                                              */
    EM_PPC64            = 21,       /* 64-bit PowerPC                                       */
    EM_V800             = 36,       /* NEC V800                                             */
    EM_FR20             = 37,       /* Fujitsu FR20                                         */
    EM_RH32             = 38,       /* TRW RH-32                                            */
    EM_RCE              = 39,       /* Motorola RCE                                         */
    EM_ARM              = 40,       /* Advanced RISC Machines ARM                           */
    EM_ALPHA            = 41,       /* Digital Alpha                                        */
    EM_SH               = 42,       /* Hitachi SH                                           */
    EM_SPARCV9          = 43,       /* SPARC Version 9                                      */
    EM_TRICORE          = 44,       /* Siemens Tricore embedded processor                   */
    EM_ARC              = 45,       /* Argonaut RISC Core, Argonaut Technologies Inc.       */
    EM_H8_300           = 46,       /* Hitachi H8/300                                       */
    EM_H8_300H          = 47,       /* Hitachi H8/300H                                      */
    EM_H8S              = 48,       /* Hitachi H8S                                          */
    EM_H8_500           = 49,       /* Hitachi H8/500                                       */
    EM_IA_64            = 50,       /* Intel IA-64 processor architecture                   */
    EM_MIPS_X           = 51,       /* Stanford MIPS-X                                      */
    EM_COLDFIRE         = 52,       /* Motorola ColdFire                                    */
    EM_68HC12           = 53,       /* Motorola M68HC12                                     */
    EM_MMA              = 54,       /* Fujitsu MMA Multimedia Accelerator                   */
    EM_PCP              = 55,       /* Siemens PCP                                          */
    EM_NCPU             = 56,       /* Sony nCPU embedded RISC processor                    */
    EM_NDR1             = 57,       /* Denso NDR1 microprocessor                            */
    EM_STARCORE         = 58,       /* Motorola Star*Core processor                         */
    EM_ME16             = 59,       /* Toyota ME16 processor                                */
    EM_ST100            = 60,       /* STMicroelectronics ST100 processor                   */
    EM_TINYJ            = 61,       /* Advanced Logic Corp. TinyJ embedded processor family */
    EM_X86_64           = 62,       /* AMD x86-64 architecture                              */
    EM_PDSP             = 63,       /* Sony DSP Processor                                   */
    EM_PDP10            = 64,       /* Digital Equipment Corp. PDP-10                       */
    EM_PDP1             = 65,       /* Digital Equipment Corp. PDP-11                       */
    EM_FX66             = 66,       /* Siemens FX66 microcontroller                         */
    EM_ST9PLUS          = 67,       /* STMicroelectronics ST9+ 8/16 bit microcontroller     */
    EM_ST7              = 68,       /* STMicroelectronics ST7 8-bit microcontroller         */
    EM_68HC16           = 69,       /* Motorola MC68HC16 Microcontroller                    */
    EM_68HC11           = 70,       /* Motorola MC68HC11 Microcontroller                    */
    EM_68HC08           = 71,       /* Motorola MC68HC08 Microcontroller                    */
    EM_68HC05           = 72,       /* Motorola MC68HC05 Microcontroller                    */
    EM_SVX              = 73,       /* Silicon Graphics SVx                                 */
    EM_ST19             = 74,       /* STMicroelectronics ST19 8-bit microcontroller        */
    EM_VAX              = 75,       /* Digital VAX                                          */
    EM_CRIS             = 76,       /* Axis Communications 32-bit embedded processor        */
    EM_JAVELIN          = 77,       /* Infineon Technologies 32-bit embedded processor      */
    EM_FIREPATH         = 78,       /* Element 14 64-bit DSP Processor                      */
    EM_ZSP              = 79,       /* LSI Logic 16-bit DSP Processor                       */
    EM_MMIX             = 80,       /* Donald Knuth's educational 64-bit processor          */
    EM_HUANY            = 81,       /* Harvard University machine-independent object files  */
    EM_PRISM            = 82        /* SiTera Prism                                         */
/*  ...lots more of these are defined, but there seems little point in listing them here.   */
} Elf_MachineType;

/* Object file versions (Elf32_Ehdr.e_version) */
typedef enum
{
    EV_NONE             = 0,        /* No version               */
    EV_CURRENT          = 1         /* Current version          */
} Elf_FileVersion;

/* Size of Elf32.Ehdr.e_ident[] char array */
#define EI_NIDENT       (16)

/* Elf32.Ehdr.e_ident[] indices */
typedef enum
{
    EI_MAG0             = 0,        /* File identification      */
    EI_MAG1             = 1,        /* File identification      */
    EI_MAG2             = 2,        /* File identification      */
    EI_MAG3             = 3,        /* File identification      */
    EI_CLASS            = 4,        /* File class               */
    EI_DATA             = 5,        /* Data encoding            */
    EI_VERSION          = 6,        /* File version             */
    EI_PAD              = 7         /* Start of padding bytes   */
} Elf_IdentIndex;

/* File identification magic numbers (Elf32_Ehdr.e_ident[0..3]) */
#define ELFMAG0         (0x7f)
#define ELFMAG1         ('E')
#define ELFMAG2         ('L')
#define ELFMAG3         ('F')

/* File classes (Elf32_Ehdr.e_ident[EI_CLASS]) */
typedef enum
{
    ELFCLASSNONE        = 0,            /* Invalid class            */
    ELFCLASS32          = 1,            /* 32-bit objects           */
    ELFCLASS64          = 2             /* 64-bit objects           */
} Elf_FileClass;

/* Encoding of processor-specific data (Elf32_Ehdr.e_ident[EI_DATA]) */
typedef enum
{
    ELFDATANONE         = 0,            /* Invalid data encoding    */
    ELFDATA2LSB         = 1,            /* Little-endian encoding   */
    ELFDATA2MSB         = 2             /* Big-endian encoding      */
} Elf_DataEncoding;

/* m68k-specific flags (Elf32_Ehdr.e_flags) */
#define EF_M68K_CPU32   (0x00810000)    /* CPU32 variant            */
#define EF_M68K_M68000  (0x01000000)    /* Motorola 68000           */
#define EF_M68K_CFV4E   (0x00008000)    /* ColdFire v4              */
#define EF_M68K_FIDO    (0x02000000)    /* Fido architecture        */

#define EF_M68K_ARCH_MASK   \
    (EF_M68K_CPU32 | EF_M68K_M68000 | EF_M68K_CFV4E | EF_M68K_FIDO)


/* ELF header structure */
typedef struct
{
    u8      e_ident[EI_NIDENT]; /* Identification bytes                                         */
    u16     e_type;             /* Object file type                                             */
    u16     e_machine;          /* Machine architecture                                         */
    u32     e_version;          /* Object file version                                          */
    u32     e_entry;            /* Entry point address                                          */
    u32     e_phoff;            /* Program header table file offset                             */
    u32     e_shoff;            /* Section header table file offset                             */
    u32     e_flags;            /* Processor-specific flags                                     */
    u16     e_ehsize;           /* ELF header size                                              */
    u16     e_phentsize;        /* Size of a program header table entry                         */
    u16     e_phnum;            /* Number of program header table entries                       */
    u16     e_shentsize;        /* Size of a section header table entry                         */
    u16     e_shnum;            /* Number of section header table entries                       */
    u16     e_shstrndx;         /* Index of section-name string table in section header table   */
} __attribute__((packed)) Elf32_Ehdr;

/* Special section indices */
typedef enum
{
    SHN_UNDEF           = 0,                /* Undefined/missing section                    */
    SHN_LORESERVE       = 0xff00,           /* Lower bound of reserved indices              */
    SHN_LOPROC          = 0xff00,           /* Processor-specific semantics - lower bound   */
    SHN_HIPROC          = 0xff1f,           /* Processor-specific semantics - upper bound   */
    SHN_ABS             = 0xfff1,           /* Absolute values (immune to relocation)       */
    SHN_COMMON          = 0xfff2,           /* Common symbols                               */
    SHN_HIRESERVE       = 0xffff            /* Upper bound of reserved indices              */
} Elf_SpecialSectionIndex;

/* Section header types (Elf32_Shdr.sh_type) */
/* Can't use an enum here because ISO C restricts enumerator values to signed int32 */
#define SHT_NULL            (0)             /* Inactive header; no associated section       */
#define SHT_PROGBITS        (1)             /* Program-specific information                 */
#define SHT_SYMTAB          (2)             /* Symbols for link editing                     */
#define SHT_STRTAB          (3)             /* String table                                 */
#define SHT_RELA            (4)             /* Relocation entries with explicit addends     */
#define SHT_HASH            (5)             /* Hash table                                   */
#define SHT_DYNAMIC         (6)             /* Information for dynamic linking              */
#define SHT_NOTE            (7)             /* Notes                                        */
#define SHT_NOBITS          (8)             /* Like SHT_PROGBITS, but occupies no space     */
#define SHT_REL             (9)             /* Relocation entries without explicit addends  */
#define SHT_SHLIB           (10)            /* Reserved; unspecified semantics              */
#define SHT_DYNSYM          (11)            /* Symbols for dynamic linking                  */
#define SHT_LOPROC          (0x70000000)    /* Processor-specific semantics - lower bound   */
#define SHT_HIPROC          (0x7fffffff)    /* Processor-specific semantics - upper bound   */
#define SHT_LOUSER          (0x80000000)    /* App-specific indices - lower bound           */
#define SHT_HIUSER          (0xffffffff)    /* App-specific indices - upper bound           */

/* Section attribute flags (Elf32_Shdr.sh_flags) */
#define SHF_WRITE           (0x1)           /* Data should be writable during execution     */
#define SHF_ALLOC           (0x2)           /* Section occupies memory during execution     */
#define SHF_EXECINSTR       (0x4)           /* Section contains executable machine instrs   */
#define SHF_MASKPROC        (0xf0000000)    /* Mask for processor-specific semantics        */

/* Section header structure */
typedef struct
{
    u32     sh_name;            /* Name of section (index into section header string table)     */
    u32     sh_type;            /* Section type                                                 */
    u32     sh_flags;           /* Miscellaneous attribute flags                                */
    u32     sh_addr;            /* Memory address at which first byte of section should reside  */
    u32     sh_offset;          /* Offset from beginning of file to first byte in section       */
    u32     sh_size;            /* Size of section                                              */
    u32     sh_link;            /* Section header table index link                              */
    u32     sh_info;            /* Extra information, section-type specific                     */
    u32     sh_addralign;       /* Address alignment required for section                       */
    u32     sh_entsize;         /* For sections containing fixed-size entries: size of an entry */
} Elf32_Shdr;

/* Symbol table "undefined symbol" index */
#define STN_UNDEF           (0)             /* Undefined symbol */

/* Extractor macros for symbol type and binding attributes (Elf32_Sym.st_info) */
#define ELF32_ST_BIND(i)    ((i) >> 4)
#define ELF32_ST_TYPE(i)    ((i) & 0xf)
#define ELF32_ST_INFO(b, t) (((b) << 4) + ((t) & 0xf))

/* Symbol binding types (Elf32_Sym.st_info) */
typedef enum
{
    STB_LOCAL           = 0,        /* Local symbol - not visible outside the file          */
    STB_GLOBAL          = 1,        /* Global symbol - visible to all files being combined  */
    STB_WEAK            = 2,        /* Global symbol with lower precedence                  */
    STB_LOPROC          = 13,       /* Processor-specific semantics - lower bound           */
    STB_HIPROC          = 15        /* Processor-specific semantics - upper bound           */
} Elf_BindingType;

/* Symbol types (Elf32_Sym.st_info) */
#define STT_NOTYPE          (0)     /* Unspecified type                                     */
#define STT_OBJECT          (1)     /* Symbol is a data object (variable, array, ...)       */
#define STT_FUNC            (2)     /* Symbol is a function or other executable code        */
#define STT_SECTION         (3)     /* Symbol is a section                                  */
#define STT_FILE            (4)     /* Symbol represents a file (e.g. a source file)        */
#define STT_LOPROC          (13)    /* Processor-specific semantics - lower bound           */
#define STT_HIPROC          (15)    /* Processor-specific semantics - upper bound           */

/* Symbol table entry */
typedef struct
{
    u32     st_name;            /* Index of symbol's name in string table       */
    u32     st_value;           /* Value of symbol                              */
    u32     st_size;            /* Size of symbol, if any                       */
    u8      st_info;            /* Symbol type and binding attributes           */
    u8      st_other;           /* (no defined meaning)                         */
    u16     st_shndx;           /* Index of section in which symbol is defined  */
} __attribute__((packed)) Elf32_Sym;

/* Relocation index/type extractor macros (Elf32_Rel*.r_info) */
#define ELF32_R_SYM(i)      ((i) >> 8)
#define ELF32_R_TYPE(i)     ((u8) (i))
#define ELF32_R_INFO(s, t)  (((s) << 8) + (u8) (t))

/* Relocation types */
typedef enum
{
    R_386_NONE          = 0,        /* No relocation                                    */
    R_386_32            = 1,        /*                                                  */
    R_386_PC32          = 2,        /*                                                  */
    R_386_GOT32         = 3,        /* Offset of symbol's entry in GOT from GOT base    */
    R_386_PLT32         = 4,        /* Address of symbol's PLT entry                    */
    R_386_COPY          = 5,        /* Copy symbol's data to specified offset           */
    R_386_GLOB_DAT      = 6,        /* Set GOT entry to address of the symbol           */
    R_386_JMP_SLOT      = 7,        /* Jump table entry in PLT                          */
    R_386_RELATIVE      = 8,        /* Relative address                                 */
    R_386_GOTOFF        = 9,        /*                                                  */
    R_386_GOTPC         = 10        /*                                                  */
} Elf_RelocType;

/* Relocation entry */
typedef struct
{
    u32     r_offset;           /* Location at which to apply the relocation action     */
    u32     r_info;             /* Index of symbol table entry and type of relocation   */
} Elf32_Rel;

typedef struct
{
    u32     r_offset;           /* Location at which to apply the relocation action     */
    u32     r_info;             /* Index of symbol table entry and type of relocation   */
    s32     r_addend;           /* Constant addend used to compute the relocation value */
} Elf32_Rela;

/* Segment types (Elf32_Phdr.p_type) */
#define PT_NULL             (0)             /* Indicates that the element is unused         */
#define PT_LOAD             (1)             /* Specifies a loadable segment                 */
#define PT_DYNAMIC          (2)             /* Specifies dynamic linking information        */
#define PT_INTERP           (3)             /* Specifies the path to an interpreter         */
#define PT_NOTE             (4)             /* The location of auxiliary information        */
#define PT_SHLIB            (5)             /* Reserved; semantics unspecified              */
#define PT_PHDR             (6)             /* Location and size of program header table    */
#define PT_LOPROC           (0x70000000)    /* Processor-specific semantics - lower bound   */
#define PT_HIPROC           (0xffffffff)    /* Processor-specific semantics - upper bound   */

/* Segment flags (Elf32_Phdr.p_flags) */
#define PF_X                (0x1)           /* Execute                      */
#define PF_W                (0x2)           /* Write                        */
#define PF_R                (0x4)           /* Read                         */
#define PF_MASKOS           (0xff000000)    /* OS-specific flag mask        */
#define PF_MASKPROC         (0xf0000000)    /* Processor-specific flag mask */

/* Program header */
typedef struct
{
    u32     p_type;             /* Segment type                                         */
    u32     p_offset;           /* Segment's offset from start of file                  */
    u32     p_vaddr;            /* Virtual address of segment                           */
    u32     p_paddr;            /* Physical address of segment                          */
    u32     p_filesz;           /* Size of file image of segment; may be zero           */
    u32     p_memsz;            /* Size of memory image of segment; may be zero         */
    u32     p_flags;            /* Flags                                                */
    u32     p_align;            /* Alignment of segment in memory and file              */
} Elf32_Phdr;

s32 elf_load_exe(const void * const buf, ku32 len, exe_img_t **img);

u32 elf_is_relevant_progbits_section(ks8 * name);
u32 elf_is_relevant_nobits_section(ks8 * name);

#endif
