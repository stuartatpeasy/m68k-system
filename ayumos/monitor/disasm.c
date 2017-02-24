/*
    MC68000/68010 disassembler

    Part of the as-yet-unnamed MC68010 operating system


    (c) Stuart Wallace <stuartw@atom.net>, 2011.
*/

#include <monitor/include/disasm.h>
#include <klibc/include/stdio.h>
#include <klibc/include/string.h>
#include <klibc/include/assert.h>
#include <kernel/util/kutil.h>


/* "binary extract from word" macros - extract various fields from an instruction word */
#define BEW_DISPLACEMENT(x)     ((char) ((x) & 0xff))
#define BEW_REGISTER(x)         (((x) & 0x7000) >> 12)
#define BEW_SCALE(x)            (((x) & 0x0600) >> 9)
#define BEW_DA(x)               (((x) & 0x8000) ? 'a' : 'd')

/* Determine whether bit <y> of value <x> is set. */
#define TEST(x, y)              (((x) & (1 << (y))) ? 1 : 0)

/*
    Addressing modes used in effective addresses
*/
#define EAMODE_DN               (0) /* data register direct                                  */
#define EAMODE_AN               (1) /* address register direct                               */
#define EAMODE_AN_IND           (2) /* address register indirect                             */
#define EAMODE_AN_IND_POSTINC   (3) /* address register indirect with postincrement          */
#define EAMODE_AN_IND_PREDEC    (4) /* address register indirect with predecrement           */
#define EAMODE_AN_DISP          (5) /* address register indirect with displacement           */
#define EAMODE_AN_DISP_INDEX    (6) /* address register indirect with displacement and index */
#define EAMODE_IMM              (7) /* immediate / absolute word/long / PC+disp / PC+d+index */

/*
    Addressing sub-modes used to identify the type of "immediate" address in an effective address
*/
#define EAMODE_IMM_ABSW             (0)     /* absolute word                                */
#define EAMODE_IMM_ABSL             (1)     /* absolute long                                */
#define EAMODE_IMM_PC_DISP          (2)     /* program counter indirect with displacement   */
#define EAMODE_IMM_PC_DISP_INDEX    (3)     /* program counter indirect with disp + index   */
#define EAMODE_IMM_IMM              (4)     /* immediate value                              */

/*
    host-to-platform (HTOP_*) macros

    If the host is little-endian, this module must be compiled with -DHOST_LITTLEENDIAN -
    the HTOP_* macros will then swap the byte order of short ints and ints.  Otherwise
    the code in this module will assume a big-endian host (e.g. m68k hardware).
*/
#ifdef HOST_LITTLEENDIAN
#define HTOP_SHORT(x)           ((((x) & 0xff) << 8) | (((x) >> 8) & 0xff))
#define HTOP_INT(x)             (((x) << 24) | (((x) & 0xff00) << 8) | (((x) & 0xff0000) >> 8) | \
                                    ((x) >> 24))
#else
#define HTOP_SHORT(x)           (x)
#define HTOP_INT(x)             (x)
#endif

/* Effective address size indicator */
typedef enum ea_size
{
    ea_unsized = 0,
    ea_byte = 'b',
    ea_word = 'w',
    ea_long = 'l',
    ea_sr = 's'         /* flag used to indicate an operation on the status register */
} ea_size_t;


/* Disassembler context object */
typedef struct dis_context
{
    dis_machtype_t      machtype;
    unsigned short **   p;
    char                a1[32];
    char                a2[32];
    const char *        pf;
    unsigned short      instr;
    unsigned char       src_mode;
    unsigned char       src_reg;
    ea_size_t           size;
} dis_context_t;


static void mmu_instr(dis_context_t * const ctx);
static void fp_instr(dis_context_t * const ctx);
static char *ea(const dis_machtype_t machtype, char *str, unsigned char mode, unsigned char reg,
                unsigned short **p, const ea_size_t sz);
static void movem_regs(char *str, unsigned short regs, char mode);


static ks8 * const branches[] =
{
    "bra", "bsr", "bhi", "bls", "bcc", "bcs", "bne", "beq",
    "bvc", "bvs", "bpl", "bmi", "bge", "blt", "bgt", "ble"
};

static ks8 * const dbranches[] =
{
    "dbt",  "dbf",  "dbhi", "dbls", "dbcc", "dbcs", "dbne", "dbeq",
    "dbvc", "dbvs", "dbpl", "dbmi", "dbge", "dblt", "dbgt", "dble"
};

static ks8 * const sets[] =
{
    "st",  "sf",  "shi", "sls", "scc", "scs", "sne", "seq",
    "svc", "svs", "spl", "smi", "sge", "slt", "sgt", "sle"
};

static ks8 * const bits[] =
{
    "btst", "bchg", "bclr", "bset"
};

static ks8 * const lshifts[] =
{
    "asl", "lsl", "roxl", "rol"
};

static ks8 * const rshifts[] =
{
    "asr", "lsr", "roxr", "ror"
};

static ks8 * const misc1[] =
{
    "reset", "nop", "stop", "rte", "rtd", "rts", "trapv", "rtr"
};

static ks8 * const misc2[] =
{
    "ori", "andi", "subi", "addi", "???", "eori", "cmpi", "moves"
};

static ks8 * const mmu_branches[] =
{
    "pbbs", "pbbc", "pbls", "pblc", "pbss", "pbsc", "pbas", "pbac",
    "pbws", "pbwc", "pbis", "pbic", "pbgs", "pbgc", "pbcs", "pbcc"
};

static ks8 * const mmu_dbranches[] =
{
    "pdbbs", "pdbbc", "pdbls", "pdblc", "pdbss", "pdbsc", "pdbas", "pdbac",
    "pdbws", "pdbwc", "pdbis", "pdbic", "pdbgs", "pdbgc", "pdbcs", "pdbcc"
};

static ks8 * const mmu_sets[] =
{
    "psbs", "psbc", "psls", "pslc", "psss", "pssc", "psas", "psac",
    "psws", "pswc", "psis", "psic", "psgs", "psgc", "pscs", "pscc"
};

static ks8 * const mmu_traps[] =
{
    "ptrapbs", "ptrapbc", "ptrapls", "ptraplc", "ptrapss", "ptrapsc", "ptrapas", "ptrapac",
    "ptrapws", "ptrapwc", "ptrapis", "ptrapic", "ptrapgs", "ptrapgc", "ptrapcs", "ptrapcc"
};

static const ea_size_t disasm_sizemap[] =
{
    ea_byte, ea_word, ea_long, ea_unsized
};

static const ea_size_t move_sizemap[] =
{
    ea_unsized, ea_byte, ea_long, ea_word
};


/*
    disassemble() - disassemble the machine instruction at the address pointed to by <*p>; return
    the disassembled string through <str>.  Updates <*p> to point at the next instruction.
*/
int disassemble(const dis_machtype_t machtype, unsigned short **p, char *str)
{
    dis_context_t ctx;
    unsigned char bit7_6, dest_mode, dest_reg;
    int x;

    ctx.instr    = HTOP_SHORT(*(*p)++);
    ctx.p        = p;
    ctx.src_mode = (ctx.instr >> 3) & 0x7;
    ctx.src_reg  = ctx.instr & 0x7;
    ctx.size     = ea_unsized;
    ctx.pf       = NULL;
    ctx.machtype = machtype;

    *str = '\0';
    bit7_6 = (ctx.instr >> 6) & 3;
    dest_mode = (ctx.instr >> 6) & 0x7;
    dest_reg = (ctx.instr >> 9) & 0x7;

    for(x = 0; x < 32; ctx.a1[x] = ctx.a2[x] = 0, ++x) ;

    switch(ctx.instr >> 12)
    {
        case 0x0:
            if(TEST(ctx.instr, 8) || ((ctx.instr >> 8) & 0xf) == 8) /* static/dynamic bit / movep */
            {
                if(ctx.src_mode == 1)       /* movep */
                {
                    const unsigned char dir = TEST(ctx.instr, 7),
                                        sz = TEST(ctx.instr, 6);

                    ctx.pf = "movep";
                    ctx.size = sz ? ea_long : ea_word;

                    if(dir)             /* reg -> mem */
                    {
                        ctx.a1[0] = 'd';
                        ctx.a1[1] = '0' + dest_reg;

                        sprintf(ctx.a2, "%d(a%c)",
                                (short) HTOP_SHORT(*(*ctx.p)++), '0' + ctx.src_reg);
                    }
                    else        /* mem -> reg */
                    {
                        sprintf(ctx.a1, "%d(a%c)",
                                (short) HTOP_SHORT(*(*ctx.p)++), '0' + ctx.src_reg);

                        ctx.a2[0] = 'd';
                        ctx.a2[1] = '0' + dest_reg;
                    }
                }
                else                    /* static/dynamic bit */
                {
                    ctx.pf = bits[bit7_6];

                    if(TEST(ctx.instr, 8))  /* dynamic bit */
                    {
                        ctx.size = ctx.src_mode ? ea_byte : ea_long;

                        ctx.a1[0] = 'd';
                        ctx.a1[1] = '0' + dest_reg;
                    }
                    else                    /* static bit */
                    {
                        sprintf(ctx.a1, "#%d", HTOP_SHORT(*(*ctx.p)++) & 0xff);
                    }
                    ea(ctx.machtype, ctx.a2, ctx.src_mode, ctx.src_reg, ctx.p, ctx.size);
                }
            }
            else
            {
                /* ori / andi / subi / addi / <static bit, handled above> / eori / cmpi / moves */
                ctx.pf = misc2[(ctx.instr >> 9) & 0x7];

                if((ctx.src_mode == 7) && (ctx.src_reg == 4))       /* -> ccr/sr */
                {
                    /* TODO: only andi/eori/ori are permitted here - validate this */
                    if(TEST(ctx.instr, 6))
                    {
                        ctx.size = ea_word;
                        ctx.a2[0] = 's';
                        ctx.a2[1] = 'r';
                    }
                    else
                    {
                        ctx.size = ea_byte;
                        ctx.a2[0] = 'c';
                        ctx.a2[1] = 'c';
                        ctx.a2[2] = 'r';
                    }
                    ea(ctx.machtype, ctx.a1, EAMODE_IMM, EAMODE_IMM_IMM, ctx.p, ctx.size);
                }
                else
                {
                    ctx.size = disasm_sizemap[bit7_6];

                    if(ctx.size)
                    {
                        ea(ctx.machtype, ctx.a1, EAMODE_IMM, EAMODE_IMM_IMM, ctx.p, ctx.size);
                        ea(ctx.machtype, ctx.a2, ctx.src_mode, ctx.src_reg, ctx.p, ctx.size);
                    }
                    else
                        ctx.pf = NULL;
                }
            }
            break;

        case 0x1:
        case 0x2:
        case 0x3:
            ctx.size = move_sizemap[ctx.instr >> 12];

            if(dest_mode == 1)      /* movea */
            {
                if(ctx.size == ea_byte)
                    break;      /* movea.b is not allowed */

                ctx.pf = "movea";
            }
            else
                ctx.pf = "move";

            ea(ctx.machtype, ctx.a1, ctx.src_mode, ctx.src_reg, ctx.p, ctx.size);
            ea(ctx.machtype, ctx.a2, dest_mode, dest_reg, p, ctx.size);
            break;

        case 0x4:
            if(TEST(ctx.instr, 8))
            {
                if(bit7_6 == 2)     /* chk */
                {
                    ctx.pf = "chk";
                    ctx.size = ea_word;
                    ctx.a2[0] = 'd';
                }
                else                /* lea */
                {
                    ctx.pf = "lea";
                    ctx.size = ea_long;
                    ctx.a2[0] = 'a';
                }
                ea(ctx.machtype, ctx.a1, ctx.src_mode, ctx.src_reg, ctx.p, ctx.size);
                ctx.a2[1] = '0' + dest_reg;
            }
            else
            {
                if(ctx.src_mode && (bit7_6 & 2) && ((dest_reg & 5) == 4))       /* movem */
                {
                    ctx.pf = "movem";
                    ctx.size = TEST(ctx.instr, 6) ? ea_long : ea_word;
                    if(dest_reg == 4)       /* movem regs -> <ea> */
                    {
                        movem_regs(ctx.a1, HTOP_SHORT(*(*ctx.p)++), ctx.src_mode == 4);
                        ea(ctx.machtype, ctx.a2, ctx.src_mode, ctx.src_reg, ctx.p, ea_long);
                    }
                    else                    /* movem <ea> -> regs */
                    {
                        movem_regs(ctx.a2, HTOP_SHORT(*(*ctx.p)++), ctx.src_mode == 4);
                        ea(ctx.machtype, ctx.a1, ctx.src_mode, ctx.src_reg, ctx.p, ea_long);
                    }
                    break;
                }

                if(bit7_6 == 3)
                {
                    switch(dest_reg)
                    {
                        case 0:     /* move from sr */
                            ctx.pf = "move";
                            ctx.size = ea_word;
                            ctx.a1[0] = 's';
                            ctx.a1[1] = 'r';
                            ea(ctx.machtype, ctx.a2, ctx.src_mode, ctx.src_reg, ctx.p, ea_long);
                            break;

                        case 1:     /* move from ccr */
                            ctx.pf = "move";
                            ctx.size = ea_word;
                            ctx.a1[0] = 'c';
                            ctx.a1[1] = 'c';
                            ctx.a1[2] = 'r';
                            ea(ctx.machtype, ctx.a2, ctx.src_mode, ctx.src_reg, ctx.p, ea_long);
                            break;

                        case 2:     /* move to ccr */
                            ctx.pf = "move";
                            ctx.size = ea_word;
                            ea(ctx.machtype, ctx.a1, ctx.src_mode, ctx.src_reg, ctx.p, ea_long);
                            ctx.a2[0] = 'c';
                            ctx.a2[1] = 'c';
                            ctx.a2[2] = 'r';
                            break;

                        case 3:     /* move to sr */
                            ctx.pf = "move";
                            ctx.size = ea_word;
                            ea(ctx.machtype, ctx.a1, ctx.src_mode, ctx.src_reg, ctx.p, ea_long);
                            ctx.a2[0] = 's';
                            ctx.a2[1] = 'r';
                            break;

                        case 4:     /* ext.l */
                            ctx.pf = "ext";
                            ctx.size = ea_long;
                            ctx.a1[0] = 'd';
                            ctx.a1[1] = '0' + ctx.src_reg;
                            break;

                        case 5:     /* tas / illegal */
                            if((ctx.src_mode == 7) && (ctx.src_reg == 4))
                                ctx.pf = "illegal";
                            else
                            {
                                ctx.pf = "tas";
                                ctx.size = ea_byte;
                                ea(ctx.machtype, ctx.a1, ctx.src_mode, ctx.src_reg, ctx.p, ea_long);
                            }
                            break;

                        case 7:     /* jmp */
                            ctx.pf = "jmp";
                            ea(ctx.machtype, ctx.a1, ctx.src_mode, ctx.src_reg, ctx.p, ea_long);
                            break;
                    }
                }
                else
                {
                    ctx.size = disasm_sizemap[bit7_6];

                    switch(dest_reg)
                    {
                        case 0:     /* negx */
                            ctx.pf = "negx";
                            ea(ctx.machtype, ctx.a1, ctx.src_mode, ctx.src_reg, ctx.p, ea_long);
                            break;

                        case 1:     /* clr */
                            ctx.pf = "clr";
                            ea(ctx.machtype, ctx.a1, ctx.src_mode, ctx.src_reg, ctx.p, ea_long);
                            break;

                        case 2:     /* neg */
                            ctx.pf = "neg";
                            ea(ctx.machtype, ctx.a1, ctx.src_mode, ctx.src_reg, ctx.p, ea_long);
                            break;

                        case 3:     /* not */
                            ctx.pf = "not";
                            ea(ctx.machtype, ctx.a1, ctx.src_mode, ctx.src_reg, ctx.p, ea_long);
                            break;

                        case 4:     /* nbcd / swap / pea / ext.w */
                            if(ctx.size == ea_byte)         /* nbcd */
                            {
                                ctx.pf = "nbcd";
                                break;
                            }

                            if(!ctx.src_mode)
                            {
                                if(ctx.size == ea_word)     /* swap */
                                {
                                    ctx.pf = "swap";
                                    ctx.a1[0] = 'd';
                                    ctx.a1[1] = '0' + ctx.src_reg;
                                }
                                else                        /* ext.w */
                                {
                                    ctx.pf = "ext";
                                    ctx.size = ea_word;
                                    ctx.a1[0] = 'd';
                                    ctx.a1[1] = '0' + ctx.src_reg;
                                }
                            }
                            else if(bit7_6 == 1)            /* pea */
                            {
                                ctx.pf = "pea";
                                ctx.size = ea_long;
                                ea(ctx.machtype, ctx.a1, ctx.src_mode, ctx.src_reg, ctx.p, ea_long);
                            }
                            break;

                        case 5:     /* tst */
                            ctx.pf = "tst";
                            ea(ctx.machtype, ctx.a1, ctx.src_mode, ctx.src_reg, ctx.p, ea_long);
                            break;

                        case 7:                 /* trap / link / unlk / move -> usp /             */
                            if(bit7_6 == 1)     /* move <- usp /reset / nop / stop / rte / rtd /  */
                            {                   /* rts / trapv / rtr / movec / jsr                */
                                switch(ctx.src_mode)
                                {
                                    case 0:                     /* trap */
                                    case 1:
                                        ctx.pf = "trap";
                                        ctx.size = ea_unsized;
                                        sprintf(ctx.a1, "#%d", ctx.instr & 0xf);
                                        break;

                                    case 2:                     /* link */
                                        ctx.pf = "link";
                                        ctx.size = ea_unsized;
                                        ctx.a1[0] = 'a';
                                        ctx.a1[1] = '0' + ctx.src_reg;
                                        sprintf(ctx.a2, "#%d", (short) HTOP_SHORT(*(*ctx.p)++));
                                        break;

                                    case 3:                     /* unlk */
                                        ctx.pf = "unlk";
                                        ctx.size = ea_unsized;
                                        ctx.a1[0] = 'a';
                                        ctx.a1[1] = '0' + ctx.src_reg;
                                        break;

                                    case 4:                     /* move -> usp */
                                        ctx.pf = "move";
                                        ctx.size = ea_long;
                                        ctx.a1[0] = 'a';
                                        ctx.a1[1] = '0' + ctx.src_reg;
                                        ctx.a2[0] = 'u';
                                        ctx.a2[1] = 's';
                                        ctx.a2[2] = 'p';
                                        break;

                                    case 5:                     /* move <- usp */
                                        ctx.pf = "move";
                                        ctx.size = ea_long;
                                        ctx.a1[0] = 'u';
                                        ctx.a1[1] = 's';
                                        ctx.a1[2] = 'p';
                                        ctx.a2[0] = 'a';
                                        ctx.a2[1] = '0' + ctx.src_reg;
                                        break;

                                    case 6:                     /* reset / nop / stop / rte /   */
                                        switch(ctx.src_reg)     /* rtd / rts / trapv / rtr      */
                                        {
                                            case 2:
                                                sprintf(ctx.a1, "#%d",
                                                        (short) HTOP_SHORT(*(*ctx.p)++));
                                                /* fall through */
                                            default:
                                                ctx.size = ea_unsized;
                                                ctx.pf = misc1[ctx.src_reg];
                                                break;
                                        }
                                        break;

                                    case 7:                     /* movec */
                                        ctx.pf = "movec";
                                        ctx.size = ea_long;
                                        if(TEST(ctx.instr, 0))      /* general reg -> control reg */
                                        {
                                            ctx.a1[0] = TEST(**ctx.p, 15) ? 'a' : 'd';
                                            ctx.a1[1] = '0' + (((**ctx.p) >> 12) & 0x7);
                                            switch(**ctx.p & 0xfff)
                                            {
                                                case 0x000:
                                                    ctx.a2[0] = 's';
                                                    ctx.a2[1] = 'f';
                                                    ctx.a2[2] = 'c';
                                                    break;

                                                case 0x001:
                                                    ctx.a2[0] = 'd';
                                                    ctx.a2[1] = 'f';
                                                    ctx.a2[2] = 'c';
                                                    break;

                                                case 0x800:
                                                    ctx.a2[0] = 'u';
                                                    ctx.a2[1] = 's';
                                                    ctx.a2[2] = 'p'; 
                                                    break;

                                                case 0x801:
                                                    ctx.a2[0] = 'v';
                                                    ctx.a2[1] = 'b';
                                                    ctx.a2[2] = 'r'; 
                                                    break;

                                                default:            /* invalid register */
                                                    ctx.pf = NULL;
                                                    break;
                                            }
                                        }
                                        else                        /* control reg -> general reg */
                                        {
                                            switch(**ctx.p & 0xfff)
                                            {
                                                case 0x000:
                                                    ctx.a1[0] = 's';
                                                    ctx.a1[1] = 'f';
                                                    ctx.a1[2] = 'c';
                                                    break;

                                                case 0x001:
                                                    ctx.a1[0] = 'd';
                                                    ctx.a1[1] = 'f';
                                                    ctx.a1[2] = 'c';
                                                    break;

                                                case 0x800:
                                                    ctx.a1[0] = 'u';
                                                    ctx.a1[1] = 's';
                                                    ctx.a1[2] = 'p';
                                                    break;

                                                case 0x801:
                                                    ctx.a1[0] = 'v';
                                                    ctx.a1[1] = 'b';
                                                    ctx.a1[2] = 'r';
                                                    break;

                                                default:            /* invalid register */
                                                    ctx.pf = NULL;
                                                    break;
                                            }
                                            ctx.a2[0] = TEST(**ctx.p, 15) ? 'a' : 'd';
                                            ctx.a2[1] = '0' + (((**ctx.p) >> 12) & 0x7);
                                        }
                                        (*p)++;
                                        break;
                                }
                            }
                            else if(bit7_6 == 2)            /* jsr */
                            {
                                ctx.pf = "jsr";
                                ctx.size = ea_unsized;
                                ea(ctx.machtype, ctx.a1, ctx.src_mode, ctx.src_reg, ctx.p, ea_long);
                            }
                            break;
                    }
                }
            }
            break;

        case 0x5:
            if(bit7_6 == 3)         /* scc / dbcc */
            {
                if(ctx.src_mode == 1)       /* dbcc */
                {
                    ctx.pf = dbranches[(ctx.instr >> 8) & 0xf];

                    ctx.size = ea_word;
                    ctx.a1[0] = 'd';
                    ctx.a1[1] = '0' + ctx.src_reg;
                    sprintf(ctx.a2, "%d", (short) HTOP_SHORT(*(*ctx.p)++));
                }
                else                        /* scc */
                {
                    ctx.pf = sets[(ctx.instr >> 8) & 0xf];

                    ctx.size = ea_byte;
                    ea(ctx.machtype, ctx.a1, ctx.src_mode, ctx.src_reg, ctx.p, ea_long);
                }
            }
            else                    /* addq / subq */
            {
                ctx.pf = TEST(ctx.instr, 8) ? "subq" : "addq";
                ctx.size = disasm_sizemap[bit7_6];

                ctx.a1[0] = '#';
                ctx.a1[1] = (dest_reg == 0) ? '8' : '0' + dest_reg;
                ea(ctx.machtype, ctx.a2, ctx.src_mode, ctx.src_reg, ctx.p, ea_long);
            }
            break;

        case 0x6:                   /* bcc / bra / bsr */
            ctx.pf = branches[(ctx.instr >> 8) & 0xf];

            if(!(ctx.instr & 0xff))
            {
                ctx.size = ea_word;
                sprintf(ctx.a1, "#%d", (short) HTOP_SHORT(*(*ctx.p)++));
            }
            else
            {
                ctx.size = ea_byte;
                sprintf(ctx.a1, "#%d", (char) (ctx.instr & 0xff));
            }
            break;

        case 0x7:
            if(!TEST(ctx.instr, 8))
            {
                ctx.pf = "moveq";
                ctx.size = ea_long;

                sprintf(ctx.a1, "#%d", ctx.instr & 0xff);

                ctx.a2[0] = 'd';
                ctx.a2[1] = '0' + dest_reg;
            }
            break;

        case 0xc:                   /* and / mulu / abcd / exg / muls */
            if(dest_mode == 5)
            {
                if(ctx.src_mode == 0)           /* exg dx, dy */
                {
                    ctx.pf = "exg";
                    ctx.size = ea_long;
                    ctx.a1[0] = ctx.a2[0] = 'd';
                    ctx.a1[1] = '0' + dest_reg;
                    ctx.a2[1] = '0' + ctx.src_reg;
                    break;
                }
                else if(ctx.src_mode == 1)      /* exg ax, ay */
                {
                    ctx.pf = "exg";
                    ctx.size = ea_long;
                    ctx.a1[0] = ctx.a2[0] = 'a';
                    ctx.a1[1] = '0' + dest_reg;
                    ctx.a2[1] = '0' + ctx.src_reg;
                    break;
                }
            }
            else if((dest_mode == 6) && (ctx.src_mode == 1))
            {
                ctx.pf = "exg";
                ctx.size = ea_long;
                ctx.a1[0] = 'd';
                ctx.a1[1] = '0' + dest_reg;
                ctx.a2[0] = 'a';
                ctx.a2[1] = '0' + ctx.src_reg;
            }

        case 0x8:                   /* or / divu / sbcd / divs */
            if((dest_mode == 3) || (dest_mode == 7))
            {
                ctx.pf = ((ctx.instr >> 12) == 0x8) ? ((dest_mode == 3) ? "divu" : "divs")
                                                    : ((dest_mode == 3) ? "mulu" : "muls");
                ctx.size = ea_word;

                ea(ctx.machtype, ctx.a1, ctx.src_mode, ctx.src_reg, ctx.p, ea_word);
                ctx.a2[0] = 'd';
                ctx.a2[1] = '0' + dest_reg;
            }
            else
            {
                if((dest_mode == 4) && !(ctx.src_mode & 6)) /* abcd / sbcd */
                {
                    ctx.pf = ((ctx.instr >> 12) == 0x8) ? "sbcd" : "abcd";
                    ctx.size = ea_byte;
                    if(TEST(ctx.instr, 3))
                    {
                        ctx.a1[0] = ctx.a2[0] = '-';
                        ctx.a1[1] = ctx.a2[1] = '(';
                        ctx.a1[2] = ctx.a2[2] = 'a';
                        ctx.a1[4] = ctx.a2[4] = ')';

                        ctx.a1[3] = '0' + ctx.src_reg;
                        ctx.a2[3] = '0' + dest_reg;
                    }
                    else
                    {
                        ctx.a1[0] = ctx.a2[0] = 'd';
                        ctx.a1[1] = '0' + ctx.src_reg;
                        ctx.a2[1] = '0' + dest_reg;
                    }
                }
                else                                    /* and / or */
                {
                    ctx.pf = ((ctx.instr >> 12) == 0x8) ? "or" : "and";
                    ctx.size = disasm_sizemap[bit7_6];

                    if(TEST(ctx.instr, 8))  /* <ea>, Dn */
                    {
                        ea(ctx.machtype, ctx.a1, ctx.src_mode, ctx.src_reg, ctx.p, ctx.size);
                        ctx.a2[0] = 'd';
                        ctx.a2[1] = '0' + dest_reg;
                    }
                    else                    /* Dn, <ea> */
                    {
                        ctx.a1[0] = 'd';
                        ctx.a1[1] = '0' + dest_reg;
                        ea(ctx.machtype, ctx.a2, ctx.src_mode, ctx.src_reg, ctx.p, ctx.size);
                    }
                }
            }
            break;

        case 0x9:       /* sub / suba / subx */
        case 0xd:       /* add / adda / addx */
            if((ctx.instr & 0x00c0) == 0x00c0)  /* adda / suba */
            {
                ctx.pf = ((ctx.instr >> 12) == 0x9) ? "suba" : "adda";
                ctx.size = TEST(ctx.instr, 8) ? ea_long : ea_word;
                ea(ctx.machtype, ctx.a1, ctx.src_mode, ctx.src_reg, ctx.p, ctx.size);
                ctx.a2[0] = 'a';
                ctx.a2[1] = '0' + dest_reg;
            }
            else
            {
                ctx.size = disasm_sizemap[bit7_6];

                if((ctx.instr & 0x0130) == 0x0100)  /* addx / subx */
                {
                    ctx.pf = ((ctx.instr >> 12) == 0x9) ? "subx" : "addx";
                    if(ctx.src_mode & 0x1)
                    {
                        ctx.a1[0] = ctx.a2[0] = '-';
                        ctx.a1[1] = ctx.a2[1] = '(';
                        ctx.a1[2] = ctx.a2[2] = 'a';
                        ctx.a1[4] = ctx.a2[4] = ')';

                        ctx.a1[3] = '0' + ctx.src_reg;
                        ctx.a2[3] = '0' + dest_reg;
                    }
                    else
                    {
                        ctx.a1[0] = ctx.a2[0] = 'd';

                        ctx.a1[1] = '0' + ctx.src_reg;
                        ctx.a2[1] = '0' + dest_reg;
                    }
                }
                else                            /* add / sub */
                {
                    ctx.pf = ((ctx.instr >> 12) == 0x9) ? "sub" : "add";
                    if(TEST(ctx.instr, 8))
                    {
                        ctx.a1[0] = 'd';
                        ctx.a1[1] = '0' + dest_reg;
                        ea(ctx.machtype, ctx.a2, ctx.src_mode, ctx.src_reg, ctx.p, ctx.size);
                    }
                    else
                    {
                        ea(ctx.machtype, ctx.a1, ctx.src_mode, ctx.src_reg, ctx.p, ctx.size);
                        ctx.a2[0] = 'd';
                        ctx.a2[1] = '0' + dest_reg;
                    }
                }
            }
            break;

        case 0xb:       /* cmp / cmpa / cmpm / eor */
            if((ctx.instr & 0x00c0) == 0x00c0)  /* cmpa */
            {
                ctx.pf = "cmpa";
                ctx.size = TEST(ctx.instr, 8) ? ea_long : ea_word;
                ea(ctx.machtype, ctx.a1, ctx.src_mode, ctx.src_reg, ctx.p, ctx.size);
                ctx.a2[0] = 'a';
                ctx.a2[1] = '0' + dest_reg;
            }
            else
            {
                ctx.size = disasm_sizemap[bit7_6];

                if(TEST(ctx.instr, 8))
                {
                    if(ctx.src_mode == 1)       /* cmpm */
                    {
                        ctx.pf = "cmpm";
                        ctx.a1[0] = ctx.a2[0] = '(';
                        ctx.a1[1] = ctx.a2[1] = 'a';
                        ctx.a1[3] = ctx.a2[3] = ')';
                        ctx.a1[4] = ctx.a2[4] = '+';

                        ctx.a1[2] = '0' + ctx.src_reg;
                        ctx.a2[2] = '0' + dest_reg;
                    }
                    else                        /* eor */
                    {
                        ctx.pf = "eor";
                        ctx.a1[0] = 'd';
                        ctx.a1[1] = '0' + dest_reg;
                        ea(ctx.machtype, ctx.a2, ctx.src_mode, ctx.src_reg, ctx.p, ctx.size);
                    }
                }
                else                            /* cmp */
                {
                    ctx.pf = "cmp";
                    ea(ctx.machtype, ctx.a1, ctx.src_mode, ctx.src_reg, ctx.p, ctx.size);
                    ctx.a2[0] = 'd';
                    ctx.a2[1] = '0' + dest_reg;
                }
            }
            break;

        case 0xe:       /* shift/rotate register/memory */
            ctx.pf = (TEST(ctx.instr, 8)) ? lshifts[ctx.src_mode & 3] : rshifts[ctx.src_mode & 3];

            if(bit7_6 == 3)     /* memory */
            {
                ctx.size = ea_word;
                ea(ctx.machtype, ctx.a1, ctx.src_mode, ctx.src_reg, ctx.p, ea_long);
            }
            else                /* register */
            {
                ctx.size = disasm_sizemap[bit7_6];

                /* immediate shift or register shift? */
                ctx.a1[0] = (ctx.src_mode & 4) ? 'd' : '#';
                ctx.a1[1] = '0' + dest_reg;

                ctx.a2[0] = 'd';
                ctx.a2[1] = '0' + ctx.src_reg;
            }
            break;

        case 0xf:       /* coprocessor instructions */
            switch(dest_reg)        /* dest_reg contains CpID for F-line instructions */
            {
                case 0:             /* CpIP 0 = MMU */
                    mmu_instr(&ctx);
                    break;

                case 1:             /* CpID 1 = FPU */
                    fp_instr(&ctx);
                    break;
            }
            break;
    }


    *p = *(ctx.p);

    /* formulate instruction string */
    if(ctx.pf)
    {
        if(ctx.size)
            sprintf(str, "%s.%c", ctx.pf, ctx.size);
        else
            strcat(str, ctx.pf);


        if(*ctx.a1)
        {
            strcat(str, " ");
            strcat(str, ctx.a1);
            if(*ctx.a2)
            {
                strcat(str, ", ");
                strcat(str, ctx.a2);
            }
        }
        return 0;
    }

    strcat(str, "???");
    return 1;
}


/*
    mmu_instr() - decode an MMU instruction
    FIXME - this function needs to be completed.
*/
static void mmu_instr(dis_context_t * const ctx)
{
    ku8 bit11_8 = (ctx->instr >> 8) & 0xf;
    ku8 bit7_6 = (ctx->instr >> 6) & 0x3;

    if(bit11_8 == 0)
    {
        if(bit7_6 == 0)
        {
            ku16 word2 = *(*ctx->p)++;
            ku8 op = word2 >> 13;

            if(op == 0)
            {
                ku8 p_reg = word2 >> 10;

                if((p_reg == 2) || (p_reg == 3))
                {
                    /* PMOVE TTx            1111 0000 00mm mrrr    000p ppRF 0000 0000      */
                }
                /* PMOVE ACx            1111 0000 00mm mrrr    000p ppR0 0000 0000      */
            }
            else if(op == 1)
            {
                /* PFLUSH ('030)        1111 0000 00mm mrrr    001m mm00 MMMf ffff      */
                /* PFLUSH[A|S] (68851)  1111 0000 00mm mrrr    001M MM0m mmmf ffff      */
                /* PLOAD ('030/68851)   1111 0000 00mm mrrr    0010 00R0 000f ffff      */
                /* PVALID               1111 0000 00mm mrrr    0010 1000 0000 0RRR      */
            }
            else if(op == 2)
            {
                /* PMOVE SRP/CRP/TC/... 1111 0000 00mm mrrr    010p ppRF 0000 0000      */
            }
            else if(op == 3)
            {
                /* PMOVE MMUSR          1111 0000 00mm mrrr    0110 00R0 0000 0000      */
                /* PMOVE PSR/PCSR       1111 0000 00mm mrrr    011p ppR0 0000 0000      */
                /* PMOVE BADx/BACx      1111 0000 00mm mrrr    011p ppR0 000n nn00      */
            }
            else if(op == 4)
            {
                /* PTEST ('EC030)       1111 0000 00mm mrrr    1000 00R0 rrrf ffff      */
                /* PTEST ('030)         1111 0000 00mm mmrr    100L LLRA rrrr ffff      */
                /* PTEST (68851)        1111 0000 00mm mrrr    100L LLRA AAff ffff      */
            }
            else if(word2 == 0xa000)            /* effectively: if(op == 5) + checking */
            {
                if(ctx->src_mode < 2)
                    return;         /* Invalid instruction */

                ctx->pf = "pflushr";
                ea(ctx->machtype, ctx->a1, ctx->src_mode, ctx->src_reg, ctx->p, ea_unsized);
            }
        }
        else if(bit7_6 == 1)
        {
            ku16 word2 = *(*ctx->p)++;

            if(word2 & 0xfff0)
                return;             /* Validate second word of the instruction */

            if(ctx->src_mode == 1)                                          /* pdbcc */
            {
                ctx->size = ea_word;

                ctx->a1[0] = 'd';
                ctx->a1[1] = '0' + ctx->src_reg;
                
                sprintf(ctx->a2, "#%d", (short) HTOP_SHORT(*(*ctx->p)++));
                
                ctx->pf = mmu_dbranches[word2];
            }
            else
            {
                if((ctx->src_mode == 7) && (ctx->src_reg > 1))              /* ptrapcc */
                {
                    switch(ctx->src_reg)
                    {
                        case 2:             /* one-word operand */
                            ctx->size = ea_word;
                            sprintf(ctx->a1, "#%d", (short) HTOP_SHORT(*(*ctx->p)++));
                            break;

                        case 3:             /* two-word operand */
                            ctx->size = ea_long;
                            sprintf(ctx->a1, "#%d", HTOP_INT(*(*ctx->p)++));
                            (*ctx->p)++;
                            break;

                        case 4:             /* no operand */
                            ctx->size = ea_unsized;
                            break;

                        default:
                            return;         /* Invalid instruction */
                    }

                    ctx->pf = mmu_traps[word2];
                    return;
                }
                else                                                        /* pscc */
                    ctx->pf = mmu_sets[word2];
            }
        }
        else                                                                /* pbcc */
        {
            if(ctx->instr & 0x30)   /* Finish validating the instruction */
                return;

            ctx->pf = mmu_branches[ctx->instr & 0xf];

            if(ctx->instr & 0x40)
            {
                /* 32-bit displacement */
                ctx->size = ea_long;
                sprintf(ctx->a1, "#%d", HTOP_INT(*(*ctx->p)++));
                (*ctx->p)++;
            }
            else
            {
                /* 16-bit displacement */
                ctx->size = ea_word;
                sprintf(ctx->a1, "#%d", (short) HTOP_SHORT(*(*ctx->p)++));
            }
        }
    }
    else if(bit11_8 == 1)
    {
        switch(bit7_6)
        {
            case 0:                                                         /* psave */
                if((ctx->src_mode < 2) || (ctx->src_mode == 3)
                   || ((ctx->src_mode == 7) && (ctx->src_reg > 1)))
                    return;         /* Invalid EA */

                ctx->pf = "psave";
                ea(ctx->machtype, ctx->a1, ctx->src_mode, ctx->src_reg, ctx->p, ea_unsized);
                break;

            case 1:                                                         /* prestore */
                if((ctx->src_mode < 2) || (ctx->src_mode == 4)
                   || ((ctx->src_mode == 7) && (ctx->src_reg >= 4)))
                    return;         /* Invalid EA */

                ctx->pf = "prestore";
                ea(ctx->machtype, ctx->a1, ctx->src_mode, ctx->src_reg, ctx->p, ea_unsized);
                break;

            default:
                return;     /* Invalid instruction */
        }
    }
    else if(bit11_8 == 5)
    {
        if(bit7_6 == 0)
        {
            ku8 bit4_3 = (ctx->instr >> 3) & 0x3;

            if(ctx->instr & 0x20)
                return;                 /* Finish validating the instruction */

            switch(bit4_3)
            {
                case 0:                                                     /* pflushn */
                    ctx->pf = "pflushn";
                    break;

                case 1:                                                     /* pflush */
                    ctx->pf = "pflush";
                    break;

                case 2:                                                     /* pflushan */
                    ctx->pf = "pflushan";
                    return;

                case 3:                                                     /* pflusha */
                    ctx->pf = "pflusha";
                    return;
            }

            ctx->a1[0] = '(';
            ctx->a1[1] = 'a';
            ctx->a1[2] = '0' + ctx->src_reg;
            ctx->a1[3] = ')';
        }
        else if(bit7_6 == 1)                                                /* ptestr / ptestw */
        {
            if((ctx->instr & 0x18) != 0x08)     /* Finish validating the instruction */
                return;

            ctx->pf = (ctx->instr & 0x20) ? "ptestr" : "ptestw";

            ctx->a1[0] = '(';
            ctx->a1[1] = 'a';
            ctx->a1[2] = '0' + ctx->src_reg;
            ctx->a1[3] = ')';
        }
    }
}


/*
    fp_instr() - decode an FPU instruction
    FIXME - this function needs to be completed.
*/
static void fp_instr(dis_context_t * const ctx)
{
    const unsigned short ext = HTOP_SHORT(*(*ctx->p)++);
    const unsigned char type = (ctx->instr >> 6) & 0x7,    /* Instruction type */
            src_spec = (ext >> 10) & 0x7,
            dest_reg = (ext >> 7) & 0x7,
            opmode = ext & 0x7f,
            ext_bit15 = ext >> 15,
            rm = (ext >> 14) & 1,
            dir = (ext >> 13) & 1;

    if(type == 0)
    {
        if(!ext_bit15 && !dir)
        {
            if(!ctx->src_mode && !ctx->src_reg && rm && (src_spec == 7))
            {
                ctx->pf = "fmove";
                sprintf(ctx->a1, "cr[%02x]", opmode);
                ctx->a2[0] = 'f';
                ctx->a2[1] = 'p';
                ctx->a2[2] = '0' + dest_reg;
            }
            else
            {
                /* FIXME: fsincos needs special handling */
                /* FIXME: fabs, fadd, fdiv, fmove[m], fmul, fneg, fsqrt, fsub, fscc, fbcc, fsave,
                frestore, fdbcc, ftrapcc */
                switch(opmode)
                {
                    case 0x01:      ctx->pf = "fint";       break;
                    case 0x02:      ctx->pf = "fsinh";      break;
                    case 0x03:      ctx->pf = "fintrz";     break;
                    case 0x06:      ctx->pf = "flognp1";    break;
                    case 0x08:      ctx->pf = "fetoxm1";    break;
                    case 0x09:      ctx->pf = "ftanh";      break;
                    case 0x0a:      ctx->pf = "fatan";      break;
                    case 0x0c:      ctx->pf = "fasin";      break;
                    case 0x0d:      ctx->pf = "fatanh";     break;
                    case 0x0e:      ctx->pf = "fsin";       break;
                    case 0x0f:      ctx->pf = "ftan";       break;
                    case 0x10:      ctx->pf = "fetox";      break;
                    case 0x11:      ctx->pf = "ftwotox";    break;
                    case 0x12:      ctx->pf = "ftentox";    break;
                    case 0x14:      ctx->pf = "flogn";      break;
                    case 0x15:      ctx->pf = "flog10";     break;
                    case 0x16:      ctx->pf = "flog2";      break;
                    case 0x19:      ctx->pf = "fcosh";      break;
                    case 0x1c:      ctx->pf = "facos";      break;
                    case 0x1d:      ctx->pf = "fcos";       break;
                    case 0x1e:      ctx->pf = "fgetexp";    break;
                    case 0x1f:      ctx->pf = "fgetman";    break;
                    case 0x21:      ctx->pf = "fmod";       break;
                    case 0x24:      ctx->pf = "fsgldiv";    break;
                    case 0x25:      ctx->pf = "frem";       break;
                    case 0x26:      ctx->pf = "fscale";     break;
                    case 0x27:      ctx->pf = "fsglmul";    break;
                    case 0x38:      ctx->pf = "fcmp";       break;
                    case 0x3a:      ctx->pf = "ftst";       break;
                }
            }
        }
    }
}


/*
    ea() - decode an effective address into a string
*/
static char *ea(const dis_machtype_t machtype, char *str, unsigned char mode, unsigned char reg,
                unsigned short **p, const ea_size_t sz)
{
    UNUSED(machtype);

    mode &= 0x7;
    reg &= 0x7;

    switch(mode)
    {
        case 0:     /* data register direct */
            str[0] = 'd';
            str[1] = '0' + reg;
            break;

        case 1:     /* address register direct */
            str[0] = 'a';
            str[1] = '0' + reg;
            break;

        case 3:     /* address register indirect with postincrement */
            str[4] = '+';
        case 2:     /* address register indirect */
            str[0] = '(';
            str[1] = 'a';
            str[2] = '0' + reg;
            str[3] = ')';
            break;

        case 4:     /* address register indirect with predecrement */
            str[0] = '-';
            str[1] = '(';
            str[2] = 'a';
            str[3] = '0' + reg;
            str[4] = ')';
            break;

        case 5:     /* address register indirect with displacement */
            sprintf(str, "%d(a%d)", (short) HTOP_SHORT(*((*p)++)), reg);
            break;

        case 6:     /* address register indirect with index */
            sprintf(str, "%d(a%d, %c%d)", BEW_DISPLACEMENT(**p),
                        reg, BEW_DA(**p), BEW_REGISTER(**p));
            (*p)++;
            break;

        case 7:     /* absolute / program counter + displacement / immediate or status register */
            switch(reg)
            {
                case 0:     /* absolute short */
                    sprintf(str, "%x", HTOP_SHORT(**p));
                    (*p)++;
                    break;

                case 1:     /* absolute long */
                    sprintf(str, "%x", HTOP_INT(*((unsigned int *) *p)));
                    *p += 2;
                    break;

                case 2:     /* program counter with displacement */
                    sprintf(str, "%d(pc)", (short) HTOP_SHORT(**p));
                    (*p)++;
                    break;

                case 3:     /* program counter with index */
                    sprintf(str, "%d(pc, %c%d)", BEW_DISPLACEMENT(**p),
                                BEW_DA(**p), BEW_REGISTER(**p));
                    (*p)++;
                    break;

                default:        /* immediate or status register */
                    switch(sz)
                    {
                        case ea_byte:
                            sprintf(str, "#%d", (char) (HTOP_SHORT(*((*p)++)) & 0xff));
                            break;

                        case ea_word:
                            sprintf(str, "#%d", (short) HTOP_SHORT(*((*p)++)));
                            break;

                        case ea_long:
                            sprintf(str, "#%d", (int) HTOP_INT(*((int *) *p)));
                            *p += 2;
                            break;

                        case ea_sr:
                            str[0] = 's';
                            str[1] = 'r';
                            break;

                        default:
                            strcat(str, "???");
                            break;
                    }
                    break;
            }
            break;
    }

    return str;
}


/*
    movem_regs() - decode a MOVEM register list into a string
*/
static void movem_regs(char *str, unsigned short regs, char mode)
{
    char c, type, any = 0;
    char current_bit, prev_bit;

    if(mode)
    {
        unsigned short r2 = 0;
        for(c = 16; c; --c, regs >>= 1)
            r2 = (r2 << 1) | (regs & 1);
        regs = r2;
    }

    for(type = 'd'; type >= 'a'; type -= 'd' - 'a')
    {
        char hyphen = 0;
        if(regs & 0xff)
        {
            if(any)
                *str++ = '/';

            for(c = prev_bit = any = 0; c < 8; regs >>= 1, ++c)
            {
                current_bit = regs & 1;
                if(current_bit && !prev_bit)
                {
                    if(any)
                        *str++ = ',';
                    *str++ = type;
                    *str++ = '0' + c;
                    any = 1;
                }
                else if(!current_bit && prev_bit)
                {
                    if(hyphen)
                    {
                        *str++ = type;
                        *str++ = '0' + c - 1;
                    }
                    any = 1;
                }
                else if(current_bit && prev_bit)
                {
                    if(!hyphen)
                        *str++ = '-';
                    hyphen = 1;
                }
                prev_bit = current_bit;
            }

            if(current_bit && hyphen)
            {
                *str++ = type;
                *str++ = '7';
            }
        }
        else regs >>= 8;
    }
}

