/*
    Interrupt handler for MC68010 CPU

    Part of ayumos


    (c) Stuart Wallace <stuartw@atom.net>, September 2015.

    NOTE: this code WILL NOT WORK CORRECTLY on a MC68000!
*/

#ifndef TARGET_MC68010
#error This code requires the MC68010 architecture
#endif


/*
    On entry, the stack will look like this:

                    15                     0
                    | ..other junk, maybe..|
                    +------+---------------+
        SP +6       |format| vector offset |
                    +------+---------------+
           +4       | program counter low  |
           +2       | program counter high |
                    +----------------------+
        SP +0       |    status register   |
                    +----------------------+

    We need to end up making a function call like this:

        some_handler(u16 irql, regs_t *regs);

    Hence the stack needs to end up looking like this, immediately before we jsr to some_handler():

                    15                     0
                    | ..other junk, maybe..|
                    +------+---------------+
                    |format| vector offset |
                    +------+---------------+
                    | program counter low  |
                    | program counter high |
                    +----------------------+
                    |    status register   |
                    +----------------------+
                    |                      |
                    .                      .
                    .   <regs_t struct>    .
                    .                      .
                    |                      |
                    +----------------------+
        SP +0       |         irql         |
                    +----------------------+

*/

irq_handler:
    movew       %sp@, %sp@-
    movel       %sp@(4), %sp@-
    moveml      %d0-%d7/%a0-%a7, %sp@-
    movel       %usp, %a0
    movel       %a0, %sp@(60)
    clrl        %d0
    movew       %sp@(76), %d0
    andiw       #0xfff, %d0
    movel       %d0, %a0
    asrw        #2, %d0
    movew       %d0, %sp@-
    addal       g_irq_handlers, %a0
    jsr         %a0@
    addl        #72, %sp
    rte
