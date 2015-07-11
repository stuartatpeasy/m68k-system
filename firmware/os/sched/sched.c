/*
    Scheduler

    Part of the as-yet-unnamed MC68010 operating system


    (c) Stuart Wallace <stuartw@atom.net>, July 2015.
*/

#include "sched/sched.h"


proc_t g_ps[64];

#define NPROC_STRUCTS (sizeof(g_ps) / sizeof(g_ps[0]))
#define for_each_proc_struct(p) \
            for(p = g_ps; p < &(g_ps[NPROC_STRUCTS]); ++p)

volatile proc_t *g_current_proc = NULL;
pid_t g_next_pid;
u32 g_ncontext_switches = 0;


void sched_init(void)
{
    proc_t *p;

    for_each_proc_struct(p)
        p->state = ps_unborn;

    g_current_proc = &(g_ps[0]);
    g_next_pid = 1;
}


pid_t create_process(const s8* name, proc_main_t main_fn, u32 *arg, ku16 flags)
{
    proc_t *p;
    pid_t retval = -1;      /* -1 indicates failure */

    cpu_disable_interrupts();

    for_each_proc_struct(p)
    {
        /* Find a vacant task_struct */
        if(p->state == ps_unborn)
        {
            s8 i;

            /* TODO: work out how best to store name */
            /* FIXME - allocate this space in user RAM */
            u8 *process_stack = (u8 *) umalloc(PROC_STACK_SIZE);

            u32 *process_stack_top = (u32 *) (process_stack + PROC_STACK_SIZE);

printf("umalloc()ed process stack at %p\n", process_stack);
printf("stack top = %p\n", process_stack_top);
            p->id = g_next_pid++;
            p->state = ps_runnable;
            p->parent = NULL;
            p->next = NULL;

            /* Register arrays might still contain values from a previous process; clear them */
            bzero(&(p->regs), sizeof(p->regs));

            /* Set up the process's initial stack */
            *(--process_stack_top) = (u32) arg;
            *(--process_stack_top) = (u32) process_end;

            p->regs.a[7] = (u32) process_stack_top;
            p->regs.pc = (u32) main_fn;

            retval = p->id;
            break;
        }
    }

    cpu_enable_interrupts();

    return retval;
}


void process_end(void)
{
    /*
        If a process's main_fn returns, this is where we'll end up.  Note that we may be in user
        mode when this fn is called, so it's not possible to do anything here that requires
        supervisor privilege
    */

    /* TODO - probably TRAP here, passing the process's ID to the handler. */
}


void irq_schedule(void)
{
    /*
        This is the interrupt handler for the system timer interrupt, which triggers a context
        switch.  This function performs the context switch.

        This function is marked as __attribute__((interrupt_handler)), which results in this
        instruction being emitted by the compiler on entry:

            movem.l d0-d1/a0-a1, -(sp)

        Having stacked these four registers, the stack pointer ends up 16 bytes below the location
        of the interrupt stack frame.  The stack frame should therefore look like this:

            SP +22        (...other junk...)        \
               +18        program counter ("PC")    | interrupt stack frame
               +16        status register ("SR")    /

               +12        a1                        \
               + 8        a0                        | placed here by
               + 4        d1                        | movem.l instruction
            SP + 0        d0                        /

        Note that register a0 is clobbered by our code almost immediately after the movem.l
        instruction.  We therefore retrieve its value from the stack and store it in
        g_current_task->regs.a0 explicitly.
    */

    cpu_disable_interrupts();   /* Not sure whether disable/enable IRQs is necessary */

    /* Store the outgoing process's state in *g_current_proc. */
    asm volatile
    (
        "movew %%sp@(16), %0@               \n"      /*   regs.sr = SR                     */
        "movel %%sp@(18), %0@-              \n"      /*   regs.pc = *(SP + 18)             */
        "movel %%usp, %%a1                  \n"      /* } regs.a7 = USP                    */
        "movel %%a1, %0@-                   \n"      /* }                                  */
        "moveml %%d0-%%d7/%%a0-%%a6, %0@-   \n"      /*   regs.[d0-d7,a0-a6] = d0-d7,a0-a6 */
        "movel %%sp@(8), %0@(32)            \n"      /*   regs.a0 = *(SP + 8)              */
        "movel %%sp@(12), %0@(36)           \n"      /*   regs.a1 = *(SP - 12)             */
        : /* no output operands */
        : "a" (&(g_current_proc->regs.sr))
        : "memory", "cc"
	);

	/*
        TODO: Decide which process to run next, and update g_current_proc accordingly
    */

    ++g_ncontext_switches;

    /*
        Start the next timeslice.  We need to do this before restoring the incoming tasks's state
        because the function call might interfere with register values.  This means that the next
        timeslice starts before the corresponding task is actually ready to run.
    */
    duart_start_counter();

    /*
        Restore the incoming task's state from g_current_task.

        Note that when we exit this function, code generated by the compiler will restore registers
        a0-a1 and d0-d1 from the stack before issuing an RTE.  We therefore overwrite these
        registers' values on the stack with values from the incoming task's state.  We also
        overwrite the return address on the stack with the incoming task's program counter value.
    */

    asm volatile
    (
        "movel %0@+, %%sp@                  \n"      /*   *(SP)      = regs.d0             */
        "movel %0@+, %%sp@(4)               \n"      /*   *(SP + 4)  = regs.d1             */
        "moveml %0@+, %%d2-%%d7             \n"      /*   d2-d7      = regs.d[2-7]         */
        "movel %0@+, %%sp@(8)               \n"      /*   *(SP + 8)  = regs.a0             */
        "movel %0@+, %%sp@(12)              \n"      /*   *(SP + 12) = regs.a1             */
        "moveml %0@+, %%a2-%%a6             \n"      /*   a2-a6      = regs.a[2-6]         */
        "movel %0@+, %%a1                   \n"      /*  } USP = regs.a[7]                 */
        "movel %%a1, %%usp                  \n"      /*  }                                 */
        "movel %0@+, %%sp@(18)              \n"      /*   *(SP + 18) = PC                  */
        "movew %0@, %%ccr                   \n"      /*   *(SP + 16) = SR                  */
        : /* no output operands */
        : "a" (&(g_current_proc->regs.d[0]))
        : "memory", "cc"
    );

    cpu_enable_interrupts();   /* Not sure whether disable/enable IRQs is necessary */
}
