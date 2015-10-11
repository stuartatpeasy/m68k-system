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

        some_handler(ku32 irql, void *data, regs_t regs);

    Hence the stack needs to end up looking like this, immediately before we jsr to some_handler():
    (note that the top two elements in the regs_t struct are SR and PC, and we "borrow" these from
    the interrupt stack frame)

                    15                     0
                    | ..other junk, maybe..|  .
                    +------+---------------+  .
       SP +78       |format| vector offset |  |    Interrupt stack frame
                    +------+---------------+  |  \
          +76       | program counter low  |  |  |
          +74       | program counter high |  |  |
                    +----------------------+  |  |
          +72       |    status register   |  |  |
                    +----------------------+  /  |
                    |                      |     |
                    .       rest of        .     |  Args for interrupt handler function
                    .    regs_t struct     .     |  (regs_t is also used to save/restore CPU state)
                    .                      .     |
           +8       |                      |     |
                    +----------------------+     |
           +6       |       data low       |     |
           +4       |       data high      |     |
                    +----------------------+     |
           +2       |       irql low       |     |
        SP +0       |       irql high      |     |
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
    andiw       #0x03ff, %d0

    asll        #1, %d0
    movel       %d0, %a0
    addal       #g_interrupt_handlers, %a0

    movel       %a0@(4), %sp@-

    asrl        #3, %d0
    movel       %d0, %sp@-

    movel       %a0@, %a0
    jsr         %a0@

    addql       #8, %sp

    movel       %sp@(60), %a0
    movel       %a0, %usp
    moveml      %sp@+, %d0-%d7/%a0-%a6

    addql       #4, %sp

    rte
