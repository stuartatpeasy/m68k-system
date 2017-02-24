/*
    MC68000/68010 disassembler

    Part of the as-yet-unnamed MC68010 operating system


    (c) Stuart Wallace <stuartw@atom.net>, 2011.
*/

#include <monitor/include/disasm.h>


static void mmu_instr(ku16 instr, unsigned short **p, const char **pf, const unsigned char src_mode,
                     const unsigned char src_reg, char *a1, char *a2, ea_size_t *size);
static void fp_instr(ku16 instr, unsigned short **p, const char **pf, const unsigned char src_mode,
                     const unsigned char src_reg, char *a1, char *a2);
static char *ea(char *str, unsigned char mode, unsigned char reg, unsigned short **p,
                const ea_size_t sz);
static void movem_regs(char *str, unsigned short regs, char mode);


static ks8 * const disasm_branches[] =
{
    "bra", "bsr", "bhi", "bls", "bcc", "bcs", "bne", "beq",
    "bvc", "bvs", "bpl", "bmi", "bge", "blt", "bgt", "ble"
};

static ks8 * const disasm_dbranches[] =
{
    "dbt",  "dbf",  "dbhi", "dbls", "dbcc", "dbcs", "dbne", "dbeq",
    "dbvc", "dbvs", "dbpl", "dbmi", "dbge", "dblt", "dbgt", "dble"
};

static ks8 * const disasm_sets[] =
{
    "st",  "sf",  "shi", "sls", "scc", "scs", "sne", "seq",
    "svc", "svs", "spl", "smi", "sge", "slt", "sgt", "sle"
};

static ks8 * const disasm_bits[] =
{
    "btst", "bchg", "bclr", "bset"
};

static ks8 * const disasm_lshifts[] =
{
    "asl", "lsl", "roxl", "rol"
};

static ks8 * const disasm_rshifts[] =
{
    "asr", "lsr", "roxr", "ror"
};

static ks8 * const disasm_misc1[] =
{
    "reset", "nop", "stop", "rte", "rtd", "rts", "trapv", "rtr"
};

static ks8 * const disasm_misc2[] =
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
    "sbs", "sbc", "sls", "slc", "sss", "ssc", "sas", "sac",
    "sws", "swc", "sis", "sic", "sgs", "sgc", "scs", "scc"
};

static const ea_size_t disasm_sizemap[] =
{
    ea_byte, ea_word, ea_long, ea_unsized
};

static const ea_size_t move_sizemap[] =
{
    ea_unsized, ea_byte, ea_long, ea_word
};


int disassemble(unsigned short **p, char *str)
{
    char a1[32], a2[32];
    const char *pf = NULL;

    const unsigned short instr = HTOP_SHORT(*(*p)++);
    const unsigned char bit7_6 = (instr >> 6) & 3,
                        src_mode = (instr >> 3) & 0x7,
                        src_reg = (instr & 0x7),
                        dest_mode = (instr >> 6) & 0x7,
                        dest_reg = (instr >> 9) & 0x7;

    ea_size_t size = ea_unsized;

    *str = '\0';

    int x;
    for(x = 0; x < 32; a1[x] = a2[x] = 0, ++x) ;

    switch(instr >> 12)
    {
        case 0x0:
            if(TEST(instr, 8) || (((instr >> 8) & 0xf) == 8))       /* static/dynamic bit / movep */
            {
                if(src_mode == 1)   /* movep */
                {
                    const unsigned char dir = TEST(instr, 7),
                                        sz = TEST(instr, 6);

                    pf = "movep";
                    size = sz ? ea_long : ea_word;

                    if(dir)     /* reg -> mem */
                    {
                        a1[0] = 'd';
                        a1[1] = '0' + dest_reg;

                        sprintf(a2, "%d(a%c)", (short) HTOP_SHORT(*(*p)++), '0' + src_reg);
                    }
                    else        /* mem -> reg */
                    {
                        sprintf(a1, "%d(a%c)", (short) HTOP_SHORT(*(*p)++), '0' + src_reg);

                        a2[0] = 'd';
                        a2[1] = '0' + dest_reg;
                    }
                }
                else                /* static/dynamic bit */
                {
                    pf = disasm_bits[bit7_6];

                    if(TEST(instr, 8))  /* dynamic bit */
                    {
                        size = src_mode ? ea_byte : ea_long;

                        a1[0] = 'd';
                        a1[1] = '0' + dest_reg;
                    }
                    else                /* static bit */
                    {
                        sprintf(a1, "#%d", HTOP_SHORT(*(*p)++) & 0xff);
                    }
                    ea(a2, src_mode, src_reg, p, size);
                }
            }
            else
            {
                /* ori / andi / subi / addi / <static bit, handled above> / eori / cmpi / moves */
                pf = disasm_misc2[(instr >> 9) & 0x7];

                if((src_mode == 7) && (src_reg == 4))       /* -> ccr/sr */
                {
                    /* TODO: only andi/eori/ori are permitted here - validate this */
                    if(TEST(instr, 6))
                    {
                        size = ea_word;
                        a2[0] = 's'; a2[1] = 'r';
                    }
                    else
                    {
                        size = ea_byte;
                        a2[0] = 'c'; a2[1] = 'c'; a2[2] = 'r';
                    }
                    ea(a1, EAMODE_IMM, EAMODE_IMM_IMM, p, size);
                }
                else
                {
                    size = disasm_sizemap[bit7_6];

                    if(size)
                    {
                        ea(a1, EAMODE_IMM, EAMODE_IMM_IMM, p, size);
                        ea(a2, src_mode, src_reg, p, size);
                    }
                    else
                        pf = NULL;
                }
            }
            break;

        case 0x1:
        case 0x2:
        case 0x3:
            size = move_sizemap[instr >> 12];

            if(dest_mode == 1)      /* movea */
            {
                if(size == ea_byte)
                    break;      /* movea.b is not allowed */

                pf = "movea";
            }
            else
                pf = "move";

            ea(a1, src_mode, src_reg, p, size);
            ea(a2, dest_mode, dest_reg, p, size);
            break;

        case 0x4:
            if(TEST(instr, 8))
            {
                if(bit7_6 == 2)     /* chk */
                {
                    pf = "chk";
                    size = ea_word;
                    a2[0] = 'd';
                }
                else                /* lea */
                {
                    pf = "lea";
                    size = ea_long;
                    a2[0] = 'a';
                }
                ea(a1, src_mode, src_reg, p, size);
                a2[1] = '0' + dest_reg;
            }
            else
            {
                if(src_mode && (bit7_6 & 2) && ((dest_reg & 5) == 4))       /* movem */
                {
                    pf = "movem";
                    size = TEST(instr, 6) ? ea_long : ea_word;
                    if(dest_reg == 4)       /* movem regs -> <ea> */
                    {
                        movem_regs(a1, HTOP_SHORT(*(*p)++), src_mode == 4);
                        ea(a2, src_mode, src_reg, p, ea_long);
                    }
                    else                    /* movem <ea> -> regs */
                    {
                        movem_regs(a2, HTOP_SHORT(*(*p)++), src_mode == 4);
                        ea(a1, src_mode, src_reg, p, ea_long);
                    }
                    break;
                }

                if(bit7_6 == 3)
                {
                    switch(dest_reg)
                    {
                        case 0:     /* move from sr */
                            pf = "move";
                            size = ea_word;
                            a1[0] = 's';
                            a1[1] = 'r';
                            ea(a2, src_mode, src_reg, p, ea_long);
                            break;

                        case 1:     /* move from ccr */
                            pf = "move";
                            size = ea_word;
                            a1[0] = 'c';
                            a1[1] = 'c';
                            a1[2] = 'r';
                            ea(a2, src_mode, src_reg, p, ea_long);
                            break;

                        case 2:     /* move to ccr */
                            pf = "move";
                            size = ea_word;
                            ea(a1, src_mode, src_reg, p, ea_long);
                            a2[0] = 'c';
                            a2[1] = 'c';
                            a2[2] = 'r';
                            break;

                        case 3:     /* move to sr */
                            pf = "move";
                            size = ea_word;
                            ea(a1, src_mode, src_reg, p, ea_long);
                            a2[0] = 's';
                            a2[1] = 'r';
                            break;

                        case 4:     /* ext.l */
                            pf = "ext";
                            size = ea_long;
                            a1[0] = 'd';
                            a1[1] = '0' + src_reg;
                            break;

                        case 5:     /* tas / illegal */
                            if((src_mode == 7) && (src_reg == 4))
                                pf = "illegal";
                            else
                            {
                                pf = "tas";
                                size = ea_byte;
                                ea(a1, src_mode, src_reg, p, ea_long);
                            }
                            break;

                        case 7:     /* jmp */
                            pf = "jmp";
                            ea(a1, src_mode, src_reg, p, ea_long);
                            break;
                    }
                }
                else
                {
                    size = disasm_sizemap[bit7_6];

                    switch(dest_reg)
                    {
                        case 0:     /* negx */
                            pf = "negx";
                            ea(a1, src_mode, src_reg, p, ea_long);
                            break;

                        case 1:     /* clr */
                            pf = "clr";
                            ea(a1, src_mode, src_reg, p, ea_long);
                            break;

                        case 2:     /* neg */
                            pf = "neg";
                            ea(a1, src_mode, src_reg, p, ea_long);
                            break;

                        case 3:     /* not */
                            pf = "not";
                            ea(a1, src_mode, src_reg, p, ea_long);
                            break;

                        case 4:     /* nbcd / swap / pea / ext.w */
                            if(size == ea_byte)         /* nbcd */
                            {
                                pf = "nbcd";
                                break;
                            }

                            if(!src_mode)
                            {
                                if(size == ea_word)     /* swap */
                                {
                                    pf = "swap";
                                    a1[0] = 'd';
                                    a1[1] = '0' + src_reg;
                                }
                                else                    /* ext.w */
                                {
                                    pf = "ext";
                                    size = ea_word;
                                    a1[0] = 'd';
                                    a1[1] = '0' + src_reg;
                                }
                            }
                            else if(bit7_6 == 1)            /* pea */
                            {
                                pf = "pea";
                                size = ea_long;
                                ea(a1, src_mode, src_reg, p, ea_long);
                            }
                            break;

                        case 5:     /* tst */
                            pf = "tst";
                            ea(a1, src_mode, src_reg, p, ea_long);
                            break;

                        case 7:                 /* trap / link / unlk / move -> usp /             */
                            if(bit7_6 == 1)     /* move <- usp /reset / nop / stop / rte / rtd /  */
                            {                   /* rts / trapv / rtr / movec / jsr                */
                                switch(src_mode)
                                {
                                    case 0:                     /* trap */
                                    case 1:
                                        pf = "trap";
                                        size = ea_unsized;
                                        sprintf(a1, "#%d", instr & 0xf);
                                        break;

                                    case 2:                     /* link */
                                        pf = "link";
                                        size = ea_unsized;
                                        a1[0] = 'a';
                                        a1[1] = '0' + src_reg;
                                        sprintf(a2, "#%d", (short) HTOP_SHORT(*(*p)++));
                                        break;

                                    case 3:                     /* unlk */
                                        pf = "unlk";
                                        size = ea_unsized;
                                        a1[0] = 'a';
                                        a1[1] = '0' + src_reg;
                                        break;

                                    case 4:                     /* move -> usp */
                                        pf = "move";
                                        size = ea_long;
                                        a1[0] = 'a';
                                        a1[1] = '0' + src_reg;
                                        a2[0] = 'u';
                                        a2[1] = 's';
                                        a2[2] = 'p';
                                        break;

                                    case 5:                     /* move <- usp */
                                        pf = "move";
                                        size = ea_long;
                                        a1[0] = 'u';
                                        a1[1] = 's';
                                        a1[2] = 'p';
                                        a2[0] = 'a';
                                        a2[1] = '0' + src_reg;
                                        break;

                                    case 6:                     /* reset / nop / stop / rte /   */
                                        switch(src_reg)         /* rtd / rts / trapv / rtr      */
                                        {
                                            case 2:
                                                sprintf(a1, "#%d", (short) HTOP_SHORT(*(*p)++));
                                                /* fall through */
                                            default:
                                                size = ea_unsized;
                                                pf = disasm_misc1[src_reg];
                                                break;
                                        }
                                        break;

                                    case 7:                     /* movec */
                                        pf = "movec";
                                        size = ea_long;
                                        if(TEST(instr, 0))          /* general reg -> control reg */
                                        {
                                            a1[0] = TEST(**p, 15) ? 'a' : 'd';
                                            a1[1] = '0' + (((**p) >> 12) & 0x7);
                                            switch(**p & 0xfff)
                                            {
                                                case 0x000:
                                                    a2[0] = 's'; a2[1] = 'f'; a2[2] = 'c';  break;
                                                case 0x001:
                                                    a2[0] = 'd'; a2[1] = 'f'; a2[2] = 'c';  break;
                                                case 0x800:
                                                    a2[0] = 'u'; a2[1] = 's'; a2[2] = 'p';  break;
                                                case 0x801:
                                                    a2[0] = 'v'; a2[1] = 'b'; a2[2] = 'r';  break;
                                                default:            /* invalid register */
                                                    pf = NULL;
                                                    break;
                                            }
                                        }
                                        else                        /* control reg -> general reg */
                                        {
                                            switch(**p & 0xfff)
                                            {
                                                case 0x000:
                                                    a1[0] = 's'; a1[1] = 'f'; a1[2] = 'c';  break;
                                                case 0x001:
                                                    a1[0] = 'd'; a1[1] = 'f'; a1[2] = 'c';  break;
                                                case 0x800:
                                                    a1[0] = 'u'; a1[1] = 's'; a1[2] = 'p';  break;
                                                case 0x801:
                                                    a1[0] = 'v'; a1[1] = 'b'; a1[2] = 'r';  break;
                                                default:            /* invalid register */
                                                    pf = NULL;
                                                    break;
                                            }
                                            a2[0] = TEST(**p, 15) ? 'a' : 'd';
                                            a2[1] = '0' + (((**p) >> 12) & 0x7);
                                        }
                                        (*p)++;
                                        break;
                                }
                            }
                            else if(bit7_6 == 2)            /* jsr */
                            {
                                pf = "jsr";
                                size = ea_unsized;
                                ea(a1, src_mode, src_reg, p, ea_long);
                            }
                            break;
                    }
                }
            }
            break;

        case 0x5:
            if(bit7_6 == 3)         /* scc / dbcc */
            {
                if(src_mode == 1)       /* dbcc */
                {
                    pf = disasm_dbranches[(instr >> 8) & 0xf];

                    size = ea_word;
                    a1[0] = 'd';
                    a1[1] = '0' + src_reg;
                    sprintf(a2, "%d", (short) HTOP_SHORT(*(*p)++));
                }
                else                    /* scc */
                {
                    pf = disasm_sets[(instr >> 8) & 0xf];

                    size = ea_byte;
                    ea(a1, src_mode, src_reg, p, ea_long);
                }
            }
            else                    /* addq / subq */
            {
                pf = TEST(instr, 8) ? "subq" : "addq";
                size = disasm_sizemap[bit7_6];

                a1[0] = '#';
                a1[1] = (dest_reg == 0) ? '8' : '0' + dest_reg;
                ea(a2, src_mode, src_reg, p, ea_long);
            }
            break;

        case 0x6:                   /* bcc / bra / bsr */
            pf = disasm_branches[(instr >> 8) & 0xf];

            if(!(instr & 0xff))
            {
                size = ea_word;
                sprintf(a1, "#%d", (short) HTOP_SHORT(*(*p)++));
            }
            else
            {
                size = ea_byte;
                sprintf(a1, "#%d", (char) (instr & 0xff));
            }
            break;

        case 0x7:
            if(!TEST(instr, 8))
            {
                pf = "moveq";
                size = ea_long;

                sprintf(a1, "#%d", instr & 0xff);

                a2[0] = 'd';
                a2[1] = '0' + dest_reg;
            }
            break;

        case 0xc:                   /* and / mulu / abcd / exg / muls */
            if(dest_mode == 5)
            {
                if(src_mode == 0)           /* exg dx, dy */
                {
                    pf = "exg";
                    size = ea_long;
                    a1[0] = a2[0] = 'd';
                    a1[1] = '0' + dest_reg;
                    a2[1] = '0' + src_reg;
                    break;
                }
                else if(src_mode == 1)      /* exg ax, ay */
                {
                    pf = "exg";
                    size = ea_long;
                    a1[0] = a2[0] = 'a';
                    a1[1] = '0' + dest_reg;
                    a2[1] = '0' + src_reg;
                    break;
                }
            }
            else if((dest_mode == 6) && (src_mode == 1))
            {
                pf = "exg";
                size = ea_long;
                a1[0] = 'd';
                a1[1] = '0' + dest_reg;
                a2[0] = 'a';
                a2[1] = '0' + src_reg;
            }

        case 0x8:                   /* or / divu / sbcd / divs */
            if((dest_mode == 3) || (dest_mode == 7))
            {
                pf = ((instr >> 12) == 0x8) ? ((dest_mode == 3) ? "divu" : "divs")
                                            : ((dest_mode == 3) ? "mulu" : "muls");
                size = ea_word;

                ea(a1, src_mode, src_reg, p, ea_word);
                a2[0] = 'd';
                a2[1] = '0' + dest_reg;
            }
            else
            {
                if((dest_mode == 4) && !(src_mode & 6)) /* abcd / sbcd */
                {
                    pf = ((instr >> 12) == 0x8) ? "sbcd" : "abcd";
                    size = ea_byte;
                    if(TEST(instr, 3))
                    {
                        a1[0] = a2[0] = '-';
                        a1[1] = a2[1] = '(';
                        a1[2] = a2[2] = 'a';
                        a1[4] = a2[4] = ')';

                        a1[3] = '0' + src_reg;
                        a2[3] = '0' + dest_reg;
                    }
                    else
                    {
                        a1[0] = a2[0] = 'd';
                        a1[1] = '0' + src_reg;
                        a2[1] = '0' + dest_reg;
                    }
                }
                else                                    /* and / or */
                {
                    pf = ((instr >> 12) == 0x8) ? "or" : "and";
                    size = disasm_sizemap[bit7_6];

                    if(TEST(instr, 8))      /* <ea>, Dn */
                    {
                        ea(a1, src_mode, src_reg, p, size);
                        a2[0] = 'd';
                        a2[1] = '0' + dest_reg;
                    }
                    else                    /* Dn, <ea> */
                    {
                        a1[0] = 'd';
                        a1[1] = '0' + dest_reg;
                        ea(a2, src_mode, src_reg, p, size);
                    }
                }
            }
            break;

        case 0x9:       /* sub / suba / subx */
        case 0xd:       /* add / adda / addx */
            if((instr & 0x00c0) == 0x00c0)  /* adda / suba */
            {
                pf = ((instr >> 12) == 0x9) ? "suba" : "adda";
                size = TEST(instr, 8) ? ea_long : ea_word;
                ea(a1, src_mode, src_reg, p, size);
                a2[0] = 'a';
                a2[1] = '0' + dest_reg;
            }
            else
            {
                size = disasm_sizemap[bit7_6];

                if((instr & 0x0130) == 0x0100)  /* addx / subx */
                {
                    pf = ((instr >> 12) == 0x9) ? "subx" : "addx";
                    if(src_mode & 0x1)
                    {
                        a1[0] = a2[0] = '-';
                        a1[1] = a2[1] = '(';
                        a1[2] = a2[2] = 'a';
                        a1[4] = a2[4] = ')';

                        a1[3] = '0' + src_reg;
                        a2[3] = '0' + dest_reg;
                    }
                    else
                    {
                        a1[0] = a2[0] = 'd';

                        a1[1] = '0' + src_reg;
                        a2[1] = '0' + dest_reg;
                    }
                }
                else                            /* add / sub */
                {
                    pf = ((instr >> 12) == 0x9) ? "sub" : "add";
                    if(TEST(instr, 8))
                    {
                        a1[0] = 'd';
                        a1[1] = '0' + dest_reg;
                        ea(a2, src_mode, src_reg, p, size);
                    }
                    else
                    {
                        ea(a1, src_mode, src_reg, p, size);
                        a2[0] = 'd';
                        a2[1] = '0' + dest_reg;
                    }
                }
            }
            break;

        case 0xb:       /* cmp / cmpa / cmpm / eor */
            if((instr & 0x00c0) == 0x00c0)      /* cmpa */
            {
                pf = "cmpa";
                size = TEST(instr, 8) ? ea_long : ea_word;
                ea(a1, src_mode, src_reg, p, size);
                a2[0] = 'a';
                a2[1] = '0' + dest_reg;
            }
            else
            {
                size = disasm_sizemap[bit7_6];

                if(TEST(instr, 8))
                {
                    if(src_mode == 1)           /* cmpm */
                    {
                        pf = "cmpm";
                        a1[0] = a2[0] = '(';
                        a1[1] = a2[1] = 'a';
                        a1[3] = a2[3] = ')';
                        a1[4] = a2[4] = '+';

                        a1[2] = '0' + src_reg;
                        a2[2] = '0' + dest_reg;
                    }
                    else                        /* eor */
                    {
                        pf = "eor";
                        a1[0] = 'd';
                        a1[1] = '0' + dest_reg;
                        ea(a2, src_mode, src_reg, p, size);
                    }
                }
                else                            /* cmp */
                {
                    pf = "cmp";
                    ea(a1, src_mode, src_reg, p, size);
                    a2[0] = 'd';
                    a2[1] = '0' + dest_reg;
                }
            }
            break;

        case 0xe:       /* shift/rotate register/memory */
            pf = (TEST(instr, 8)) ? disasm_lshifts[src_mode & 3] : disasm_rshifts[src_mode & 3];

            if(bit7_6 == 3)     /* memory */
            {
                size = ea_word;
                ea(a1, src_mode, src_reg, p, ea_long);
            }
            else                /* register */
            {
                size = disasm_sizemap[bit7_6];

                /* immediate shift or register shift? */
                a1[0] = (src_mode & 4) ? 'd' : '#';
                a1[1] = '0' + dest_reg;

                a2[0] = 'd';
                a2[1] = '0' + src_reg;
            }
            break;

        case 0xf:       /* coprocessor instructions */
            switch(dest_reg)        /* dest_reg contains CpID for F-line instructions */
            {
                case 0:             /* CpIP 0 = MMU */
                    mmu_instr(instr, p, &pf, src_mode, src_reg, a1, a2, &size);
                    break;

                case 1:             /* CpID 1 = FPU */
                    fp_instr(instr, p, &pf, src_mode, src_reg, a1, a2);
                    break;
            }
            break;
    }


    /* formulate instruction string */
    if(pf)
    {
        if(size)
            sprintf(str, "%s.%c", pf, size);
        else
            strcat(str, pf);


        if(*a1)
        {
            strcat(str, " ");
            strcat(str, a1);
            if(*a2)
            {
                strcat(str, ", ");
                strcat(str, a2);
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
static void mmu_instr(ku16 instr, unsigned short **p, const char **pf, const unsigned char src_mode,
               const unsigned char src_reg, char *a1, char *a2, ea_size_t *size)
{
    ku8 bit11_8 = (instr >> 8) & 0xf;
    ku8 bit7_6 = (instr >> 6) & 0x3;
    UNUSED(a2);

    if(bit11_8 == 0)
    {
        if(bit7_6 == 0)
        {
            /* PFLUSH ('030)        1111 0000 00mm mrrr    001m mm00 MMMf ffff                          */
            /* PFLUSH[A|S] (68851)  1111 0000 00mm mrrr    001M MM0m mmmf ffff                          */
            /* PFLUSHR (68851)      1111 0000 00mm mrrr    1010 0000 0000 0000                          */
            /* PLOAD ('030/68851)   1111 0000 00mm mrrr    0010 00R0 000f ffff                          */

            /* PMOVE MMUSR          1111 0000 00mm mrrr    0110 00R0 0000 0000                          */
            /* PMOVE SRP/CRP/TC/... 1111 0000 00mm mrrr    010p ppRF 0000 0000                          */
            /* PMOVE TTx            1111 0000 00mm mrrr    000p ppRF 0000 0000                          */
            /* PMOVE ACx            1111 0000 00mm mrrr    000p ppR0 0000 0000                          */
            /* PMOVE PSR/PCSR       1111 0000 00mm mrrr    011p ppR0 0000 0000                          */
            /* PMOVE BADx/BACx      1111 0000 00mm mrrr    011p ppR0 000n nn00                          */

            /* PTEST ('030)         1111 0000 00mm mmrr    100L LLRA rrrr ffff                          */
            /* PTEST ('EC030)       1111 0000 00mm mrrr    1000 00R0 rrrf ffff                          */
            /* PTEST (68851)        1111 0000 00mm mrrr    100L LLRA AAff ffff                          */
            /* PVALID               1111 0000 00mm mrrr    0010 1000 0000 0RRR                          */
        }
        else if(bit7_6 == 1)
        {
            /* PDBcc                1111 0000 0100 1rrr    0000 0000 00cc cccc    dddd dddd dddd dddd   */
            /* PScc                 1111 0000 01mm mrrr    0000 0000 00cc cccc                          */

            /* PTRAPcc              1111 0000 0111 1MMM    0000 0000 00cc cccc   [dddd dddd dddd dddd] [dddd dddd dddd dddd] */
        }
        else                                                                /* pbcc */
        {
            if(instr & 0x30)        /* Finish validating the instruction */
                return;

            *pf = mmu_branches[instr & 0xf];

            if(instr & 0x40)
            {
                /* 32-bit displacement */
                *size = ea_long;
                sprintf(a1, "#%d", HTOP_INT(*(*p)++));
            }
            else
            {
                /* 16-bit displacement */
                *size = ea_word;
                sprintf(a1, "#%d", (short) HTOP_SHORT(*(*p)++));
            }
        }
    }
    else if(bit11_8 == 1)
    {
        switch(bit7_6)
        {
            case 0:                                                         /* psave */
                if((src_mode < 2) || (src_mode == 3) || ((src_mode == 7) && (src_reg > 1)))
                    return;         /* Invalid EA */

                *pf = "psave";
                ea(a1, src_mode, src_reg, p, ea_unsized);
                break;

            case 1:                                                         /* prestore */
                if((src_mode < 2) || (src_mode == 4) || ((src_mode == 7) && (src_reg >= 4)))
                    return;         /* Invalid EA */

                *pf = "prestore";
                ea(a1, src_mode, src_reg, p, ea_unsized);
                break;

            default:
                return;     /* Invalid instruction */
        }
    }
    else if(bit11_8 == 5)
    {
        if(bit7_6 == 0)
        {
            ku8 bit4_3 = (instr >> 3) & 0x3;

            if(instr & 0x20)
                return;                 /* Finish validating the instruction */

            switch(bit4_3)
            {
                case 0:                                                     /* pflushn */
                    *pf = "pflushn";
                    break;

                case 1:                                                     /* pflush */
                    *pf = "pflush";
                    break;

                case 2:                                                     /* pflushan */
                    *pf = "pflushan";
                    return;

                case 3:                                                     /* pflusha */
                    *pf = "pflusha";
                    return;
            }

            *a1++ = '(';
            *a1++ = 'a';
            *a1++ = '0' + src_reg;
            *a1 = ')';
        }
        else if(bit7_6 == 1)                                                /* ptestr / ptestw */
        {
            if((instr & 0x18) != 0x08)  /* Finish validating the instruction */
                return;

            *pf = (instr & 0x20) ? "ptestr" : "ptestw";

            *a1++ = '(';
            *a1++ = 'a';
            *a1++ = '0' + src_reg;
            *a1 = ')';
        }
    }
}


/*
    fp_instr() - decode an FPU instruction
    FIXME - this function needs to be completed.
*/
static void fp_instr(ku16 instr, unsigned short **p, const char **pf, const unsigned char src_mode,
                     const unsigned char src_reg, char *a1, char *a2)
{
    const unsigned short ext = HTOP_SHORT(*(*p)++);
    const unsigned char type = (instr >> 6) & 0x7,     /* Instruction type */
            src_spec = (ext >> 10) & 0x7,
            dest_reg = (ext >> 7) & 0x7,
            opmode = ext & 0x7f,
            ext_bit15 = ext >> 15,
            rm = (ext >> 14) & 1,
            dir = (ext >> 13) & 1;

    UNUSED(src_spec);

    if(type == 0)
    {
        if(!ext_bit15 && !dir)
        {
            if(!src_mode && !src_reg && rm && (src_spec == 7))
            {
                *pf = "fmove";
                sprintf(a1, "cr[%02x]", opmode);
                *a2++ = 'f';
                *a2++ = 'p';
                *a2++ = '0' + dest_reg;
            }
            else
            {
                /* FIXME: fsincos needs special handling */
                /* FIXME: fabs, fadd, fdiv, fmove[m], fmul, fneg, fsqrt, fsub, fscc, fbcc, fsave,
                frestore, fdbcc, ftrapcc */
                switch(opmode)
                {
                    case 0x01:      *pf = "fint";       break;
                    case 0x02:      *pf = "fsinh";      break;
                    case 0x03:      *pf = "fintrz";     break;
                    case 0x06:      *pf = "flognp1";    break;
                    case 0x08:      *pf = "fetoxm1";    break;
                    case 0x09:      *pf = "ftanh";      break;
                    case 0x0a:      *pf = "fatan";      break;
                    case 0x0c:      *pf = "fasin";      break;
                    case 0x0d:      *pf = "fatanh";     break;
                    case 0x0e:      *pf = "fsin";       break;
                    case 0x0f:      *pf = "ftan";       break;
                    case 0x10:      *pf = "fetox";      break;
                    case 0x11:      *pf = "ftwotox";    break;
                    case 0x12:      *pf = "ftentox";    break;
                    case 0x14:      *pf = "flogn";      break;
                    case 0x15:      *pf = "flog10";     break;
                    case 0x16:      *pf = "flog2";      break;
                    case 0x19:      *pf = "fcosh";      break;
                    case 0x1c:      *pf = "facos";      break;
                    case 0x1d:      *pf = "fcos";       break;
                    case 0x1e:      *pf = "fgetexp";    break;
                    case 0x1f:      *pf = "fgetman";    break;
                    case 0x21:      *pf = "fmod";       break;
                    case 0x24:      *pf = "fsgldiv";    break;
                    case 0x25:      *pf = "frem";       break;
                    case 0x26:      *pf = "fscale";     break;
                    case 0x27:      *pf = "fsglmul";    break;
                    case 0x38:      *pf = "fcmp";       break;
                    case 0x3a:      *pf = "ftst";       break;
                }
            }
        }
    }
}


/*
    ea() - decode an effective address into a string
*/
static char *ea(char *str, unsigned char mode, unsigned char reg, unsigned short **p,
                const ea_size_t sz)
{
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
