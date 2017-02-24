#ifndef MONITOR_DISASM_H_INC
#define MONITOR_DISASM_H_INC
/*
    MC68000/68010 disassembler

    Part of the as-yet-unnamed MC68010 operating system


    (c) Stuart Wallace, 2011.
*/

/* Machine (processor) types */
typedef enum dis_machtype
{
    mt68000,
    mt68010,
    mt68020,
    mt68020_68851,
    mt68030,
    mt68ec030,
    mt68040,
    mt68ec040,
    mt68lc040
} dis_machtype_t;

int disassemble(const dis_machtype_t machtype, unsigned short **p, char *str);

#endif
