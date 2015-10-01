/*
    Interrupt handler for MC68010 CPU

    Part of ayumos


    (c) Stuart Wallace <stuartw@atom.net>, September 2015.

    NOTE: this code WILL NOT WORK on a MC68000, because the 68000 doesn't store a vector offset in
    its interrupt stack frame.
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

    We need to call the interrupt handler in this way:

        some_handler(u16 irql, void *data, regs_t *regs);

    Hence the stack needs to end up looking like this, immediately before we jsr to some_handler():
    (note that the top two elements in the regs_t struct are SR and PC, and we "borrow" these from
    the interrupt stack frame)

                    15                     0
                    | ..other junk, maybe..|  .
                    +------+---------------+  .
       SP +76       |format| vector offset |  |    Interrupt stack frame
                    +------+---------------+  |  \
          +74       | program counter low  |  |  |
          +72       | program counter high |  |  |
                    +----------------------+  |  |
          +70       |    status register   |  |  |
                    +----------------------+  /  |
                    |                      |     |
                    .       rest of        .     |  Args for interrupt handler function
                    .    regs_t struct     .     |  (regs_t is also used to save/restore CPU state)
                    .                      .     |
           +6       |                      |     |
                    +----------------------+     |
           +4       |       data low       |     |
           +2       |       data high      |     |
                    +----------------------+     |
        SP +0       |         irql         |     |
                    +----------------------+     /

    When some_handler() returns, we need to restore CPU state and issue an RTE to resume normal
    processing.
*/

.global irq_handler

irq_handler:
    moveml      %d0-%d7/%a0-%a7, %sp@-
    movel       %usp, %a0
    movel       %a0, %sp@(60)
    clrl        %d0
    movew       %sp@(70), %d0
    andiw       #0xfff, %d0
    aslw        #1, %d0
    movel       %d0, %a0
    addal       g_interrupt_handlers, %a0
    movel       %a0@(4), %sp@-
    asrw        #3, %d0
    movew       %d0, %sp@-
    jsr         %a0@
    addl        #6, %sp
    movel       %sp@(60), %a0
    movel       %a0, %usp
    moveml      %sp@+, %d0-%d7/%a0-%a6
    addl        #4, %sp
    rte
