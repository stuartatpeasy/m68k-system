/*
    Interrupt handlers for MC68010 CPU

    Part of ayumos


    (c) Stuart Wallace <stuartw@atom.net>, September 2015.

    NOTE: this code WILL NOT WORK on a MC68000, because the 68000 doesn't store a vector offset in
    its interrupt stack frame.
*/

#ifndef TARGET_MC68010
#error This code requires the MC68010 architecture
#endif

.global irq_router_full
.global irq_router_fast
.global irq_router_swi


/*
    This module implements three interrupt routers:

        irq_router_full
            - saves/restores full CPU context before calling the interrupt handler.  Used for
              context-switching and error-reporting.

        irq_router_fast
            - saves/restores minimal context.  Used by hardware interrupt handlers.

        irq_router_swi
            - saves/restores minimal context.  Used for software interrupts (i.e. system calls).


    On entry, the stack will look like this:

                 15                     0
                 | ..other junk, maybe..|
                 +------+---------------+
        SP +6    |format| vector offset |
                 +------+---------------+
           +4    | program counter low  |
           +2    | program counter high |
                 +----------------------+
        SP +0    |    status register   |
                 +----------------------+

    "Other junk" refers to additional data stacked for bus and address error exceptions.


    irq_router_full, which stacks a complete CPU state, needs to call an interrupt handler function
    in this way:

        some_handler(ku32 irql, void *data, regs_t regs);

    Hence the stack needs to end up looking like this, immediately before we jsr to some_handler():
    (note that the top two elements in the regs_t struct are SR and PC, and we "borrow" these from
    the interrupt stack frame)

                 15                     0
                 | ..other junk, maybe..|  .
                 +------+---------------+  .
        SP +78   |format| vector offset |  |    Interrupt stack frame
                 +------+---------------+  |  \
                 | program counter low  |  |  |
           +74   | program counter high |  |  |
                 +----------------------+  |  |
           +72   |    status register   |  |  |
                 +----------------------+  /  |
                 |                      |     |
                 .       rest of        .     |  Args for interrupt handler function
                 .    regs_t struct     .     |  (regs_t is also used to save/restore CPU state)
                 .                      .     |
           +8    |                      |     |
                 +----------------------+     |
                 |       data low       |     |
           +4    |       data high      |     |
                 +----------------------+     |
                 |       irql low       |     |
        SP +0    |       irql high      |     |
                 +----------------------+     /
*/

/* "full" IRQ router: saves/restores complete CPU state.  Allows for task-switching etc. */
/* Execution time: 468 clocks on MC68010, assuming zero wait states */
irq_router_full:
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

/*
    The fast IRQ router only stacks the parts of the state which are defined as "throwaway" by the
    ABI, i.e. registers D0-D1 and A0-A1.  Any registers modified by the handler function will be
    saved/restored by the function itself.  The handler function call looks like this:

        some_fast_handler(ku32 irql, void *data);

    The required stack frame is therefore:

                 15                     0
                 | ..other junk, maybe..|  .
                 +------+---------------+  .
        SP +30   |format| vector offset |  |  Interrupt stack frame
                 +------+---------------+  |
                 | program counter low  |  |
           +26   | program counter high |  |
                 +----------------------+  |
           +24   |    status register   |  |
                 +----------------------+  /
                 |       A1 low         |  \
           +20   |       A1 high        |  |
                 +----------------------+  |
                 |       A0 low         |  |
           +16   |       A0 high        |  |
                 +----------------------+  |  Saved CPU state
                 |       D1 low         |  |
           +12   |       D1 high        |  |
                 +----------------------+  |
                 |       D0 low         |  |
           +8    |       D0 high        |  |
                 +----------------------+  /
                 |       data low       |  \
           +4    |       data high      |  |  Args for interrupt handler function
                 +----------------------+  |  (regs_t is also used to save/restore CPU state)
                 |       irql low       |  |
        SP +0    |       irql high      |  |
                 +----------------------+  /

    When some_handler() or some_fast_handler() returns, we need to restore CPU state and issue an
    RTE to resume normal processing.
*/


irq_router_fast:
    moveml      %d0-%d1/%a0-%a1, %sp@-

    movew       %sp@(22), %d0
    andiw       #0x03ff, %d0
    extwl       %d0

    asll        #1, %d0
    movel       %d0, %a0
    addal       #g_interrupt_handlers, %a0

    movel       %a0@(4), %sp@-

    asrl        #3, %d0
    movel       %d0, %sp@-

    movel       %a0@, %a0
    jsr         %a0@

    addql       #8, %sp

    moveml      %sp@+, %d0-%d1/%a0-%a1

    rte
