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

proc_t *g_current_proc = NULL;
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


pid_t create_process(const s8* name, proc_main_t main_fn, u32 *arg, ku32 stack_len, ku16 flags)
{
    proc_t *p;
    pid_t retval = -1;      /* -1 indicates failure */

    cpu_disable_interrupts();

    for_each_proc_struct(p)
    {
        /* Find a vacant task_struct */
        if(p->state == ps_unborn)
        {
            /* TODO: work out how best to store name */
            /* FIXME - allocate this space in user RAM */
            u8 *process_stack = (u8 *) umalloc(stack_len);

            u32 *process_stack_top = (u32 *) (process_stack + stack_len);

printf("umalloc()ed process stack at %p\n", process_stack);
printf("stack top = %p\n", process_stack_top);
            p->id = g_next_pid++;
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
            p->state = ps_runnable; /* Mark process as runnable so scheduler will pick it up. */
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
        supervisor privilege.

        move.l  d0, d1          // the process's retval should arrive in d0
        move.l  #SYS_exit, d0
        trap    #0
    */
    register u32 proc_retval asm("d0");
#if 0
    asm volatile
    (
        "movel %%d0, %0"
        : "=m" (&proc_retval)
        : /* no input operands */
        : "cc"
    );
#endif
    SYSCALL1(SYS_exit, proc_retval);
    /* will not return */
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

    duart_stop_counter();
    cpu_disable_interrupts();   /* Not sure whether disable/enable IRQs is necessary */

    /* Store the outgoing process's state in *g_current_proc. */
    asm volatile
    (
        "movew %%sp@(16), %0@               \n"      /*   regs.sr = SR                     */
        "movel %%sp@(18), %0@-              \n"      /*   regs.pc = *(SP + 18)             */
        "movel %%usp, %%a1                  \n"      /* } regs.a7 = USP                    */
        "movel %%a1, %0@-                   \n"      /* }                                  */
        "moveml %%a2-%%a6, %0@-             \n"      /*   regs.[a2-a6] = a2-a6             */
        "movel %%sp@(12), %0@-              \n"      /*   regs.a1 = *(SP + 12)             */
        "movel %%sp@(8), %0@-               \n"      /*   regs.a0 = *(SP + 8)              */
        "moveml %%d2-%%d7, %0@-             \n"      /*   regs.[d2-d7] = d2-d7             */
        "movel %%sp@(4), %0@-               \n"      /*   regs.d1 = *(SP + 4)              */
        "movel %%sp@, %0@-                  \n"      /*   regs.d0 = *(SP + 0)              */
        : /* no output operands */
        : "a" (&(g_current_proc->regs.sr))
        : "memory", "cc", "a1"
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
        "movew %0@, %%sp@(16)               \n"      /*   *(SP + 16) = SR                  */
        : /* no output operands */
        : "a" (&(g_current_proc->regs.d[0]))
        : "memory", "cc", "a1"
    );

    cpu_enable_interrupts();   /* Not sure whether disable/enable IRQs is necessary */
}


void sched_dump_proc(proc_t *ps)
{
    printf("D0=%08x  D1=%08x  D2=%08x  D3=%08x\n"
           "D4=%08x  D5=%08x  D6=%08x  D7=%08x\n"
           "A0=%08x  A1=%08x  A2=%08x  A3=%08x\n"
           "A4=%08x  A5=%08x  A6=%08x  SP=%08x\n\n"
           "PC=%08x  SR=%04x  [%s]\n",
           ps->regs.d[0], ps->regs.d[1], ps->regs.d[2], ps->regs.d[3],
           ps->regs.d[4], ps->regs.d[5], ps->regs.d[6], ps->regs.d[7],
           ps->regs.a[0], ps->regs.a[1], ps->regs.a[2], ps->regs.a[3],
           ps->regs.a[4], ps->regs.a[5], ps->regs.a[6], ps->regs.a[7],
           ps->regs.pc, ps->regs.sr, cpu_dump_status_register(ps->regs.sr));
}
