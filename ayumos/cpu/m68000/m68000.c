/*
    Implementations of MC68000-specific functions

    Part of ayumos


    (c) Stuart Wallace <stuartw@atom.net>, September 2015.
*/

#include <cpu/m68000/m68000.h>
#include <kernel/include/ksym.h>
#include <kernel/include/process.h>
#include <kernel/include/memory/extents.h>
#include <kernel/util/kutil.h>
#include <klibc/include/stdio.h>


void mc68000_bus_error_handler(void *dummy);
void mc68000_address_error_handler(void *dummy);
const char * mc68000_dump_status_register(ku16 sr);
void mc68000_dump_regs(const regs_t *regs);
void mc68010_dump_address_exc_frame(const mc68010_address_exc_frame_t * const aef);
const char * mc68000_dump_ssw(ku16 ssw);
const char * mc68000_dump_fc(ku16 fc);
void mc68000_try_stack_dump(ku32 sp, u32 nbytes);


char g_errsym[128];


/*
    Initialise the MC68000 vector table to some safe defaults.  This should ensure that any IRQs
    are handled correctly, by being ignored, or reported and ignored, or being reported and
    followed by a CPU halt.
*/
void cpu_irq_init_arch_specific(void)
{
    u16 u;

    /* Set magic numbers in the bottom two vectors - these help (a bit) with debugging */
    CPU_EXC_VPTR_SET(V_ssp,             0xca5caded);                /* nonsense number */
    CPU_EXC_VPTR_SET(V_reset,           0xbed51de5);                /* nonsense number */

    /* Bus and address errors have their own handler */
    CPU_EXC_VPTR_SET(V_bus_error,       mc68000_bus_error_handler);
    CPU_EXC_VPTR_SET(V_address_error,   mc68000_address_error_handler);

    /* Point all other exception vectors at the generic IRQ handler code initially */
    for(u = V_illegal_instruction; u <= CPU_MAX_IRQL; ++u)
        CPU_EXC_VPTR_SET(u, irq_router_fast);

    /* System calls use TRAP #0 */
    CPU_EXC_VPTR_SET(V_trap_0, syscall_dispatcher);
}


/*
    mc68000_bus_error_handler() - report a bus error and halt the system.
*/
void mc68000_bus_error_handler(void *dummy)
{
    const mc68010_address_exc_frame_t * const aef =
        (mc68010_address_exc_frame_t *) ((u32) &dummy - 4);

    cpu_disable_interrupts();

    ksym_format_nearest_prev((void *) aef->pc, g_errsym, sizeof(g_errsym));

    printf("\nBus error in process %d\n\n"
           "PC=%08x  %s\nSR=%04x  [%s]\n\n",
           proc_get_pid(), aef->pc, g_errsym, aef->sr, mc68000_dump_status_register(aef->sr));

    mc68010_dump_address_exc_frame(aef);

    cpu_halt();
}


/*
    mc68000_address_error_handler() - report an address error and halt the system.
*/
void mc68000_address_error_handler(void *dummy)
{
    const mc68010_address_exc_frame_t * const aef =
        (mc68010_address_exc_frame_t *) ((u32) &dummy - 4);

    cpu_disable_interrupts();

    ksym_format_nearest_prev((void *) aef->pc, g_errsym, sizeof(g_errsym));

    printf("\nAddress error in process %d\n\n"
           "PC=%08x  %s\nSR=%04x  [%s]\n\n",
           proc_get_pid(), aef->pc, g_errsym, aef->sr, mc68000_dump_status_register(aef->sr));

    mc68010_dump_address_exc_frame(aef);

    cpu_halt();
}


/*
    cpu_default_irq_handler() - handler for interrupts not otherwise handled.  Dumps state and halts
    the CPU.
*/
void cpu_default_irq_handler(ku32 irql, void *data)
{
    const proc_t * const proc = proc_current();
    const regs_t * const regs = &proc->regs;
    UNUSED(data);

    cpu_disable_interrupts();

    printf("\n\nUnhandled exception in process %d: ", proc_get_pid());
    if((irql >= 25) && (irql <= 31))
        printf("Level %d interrupt autovector (vector %d)\n", irql - 24, irql);
    else if((irql >= 32) && (irql <= 47))
        printf("Trap #%d (vector %d)\n", irql - 32, irql);
    else if(irql >= 64)
        printf("User-defined interrupt %d (vector %d)\n", irql - 64, irql);
    else
    {
        const char *msg = NULL;
        switch(irql)
        {
            case 4:  msg = "Illegal instruction";                   break;
            case 5:  msg = "Integer divide by zero";                break;
            case 6:  msg = "CHK/CHK2 instruction";                  break;
            case 7:  msg = "FTRAPcc/TRAPcc/TRAPV instruction";      break;
            case 8:  msg = "Privilege violation";                   break;
            case 9:  msg = "Trace";                                 break;
            case 10: msg = "Line 1010 emulator";                    break;
            case 11: msg = "Line 1111 emulator";                    break;
            case 13: msg = "Coprocessor protocol violation";        break;
            case 14: msg = "Format error";                          break;
            case 15: msg = "Unitialised interrupt";                 break;
            case 24: msg = "Spurious interrupt";                    break;
            case 48: msg = "FP branch/set on unordered condition";  break;
            case 49: msg = "FP inexact result";                     break;
            case 50: msg = "FP divide by zero";                     break;
            case 51: msg = "FP underflow";                          break;
            case 52: msg = "FP operand error";                      break;
            case 53: msg = "FP overflow";                           break;
            case 54: msg = "FP signalling NaN";                     break;
            case 55: msg = "FP unimplemented data type";            break;
            case 56: msg = "MMU configuration error";               break;
            case 57: msg = "MMU illegal operation";                 break;
            case 58: msg = "MMU access level violation";            break;
            default: msg = "Unknown interrupt";                     break;
        }

        printf("%s (vector %d)\n", msg, irql);
    }

    putchar('\n');
    mc68000_dump_regs(regs);

    printf("\n-- Supervisor stack (base: %p) --\n", proc->kstack);
    mc68000_try_stack_dump(regs->a[7], 16);

    printf("\n-- User stack (base: %p) --\n", proc->ustack);
    mc68000_try_stack_dump(regs->usp, 16);

    cpu_halt();
}


/*
    cpu_halt() - stop processing.
*/
void cpu_halt(void)
{
    puts("\nSystem halted.");

    /* the arg to "stop" causes the CPU to stay in supervisor mode and sets the IRQ mask to 7 */
    asm volatile
    (
        "stop       #0x2700             \n"
    );

    /* Won't return */
    while(1) ;
}


/*
    cpu_swi() - raise a software interrupt.
*/
u32 cpu_swi(ku32 num)
{
    register u32 ret asm("d0");

    asm volatile
    (
        "moveml     %%d1/%%a0-%%a2, %%sp@-  \n"
        "movel      %0, %%a0                \n" /* FIXME - calculated stack offset is wrong */
        "trap       #15                     \n"
        "moveml     %%sp@+, %%d1/%%a0-%%a2  \n"
        :
        : "m" (num) /* FIXME - the calculated address of this var is wrong following the movem */
        : "cc"
    );

    return ret;
}


/*
    cpu_proc_init() - perform architecture-specific register initialisation before a new process
    starts.
*/
s32 cpu_proc_init(regs_t *r, void *entry_point, void *arg, void *ustack_top, void *kstack_top,
                  ku32 flags)
{
    u32 *sp;
    mc68010_short_exc_frame_t *f;

    /* MC680x0 requires that the PC and SP are aligned to an even-numbered address */
    if(((u32) entry_point & 1) || (((u32) kstack_top) & 1) || (((u32) ustack_top) & 1))
        return -EINVAL;

    /* Set flags and store the argument to the process on the correct stack. */
    if(flags & PROC_TYPE_KERNEL)
    {
        sp = (u32 *) kstack_top;
        *--sp = (u32) arg;
        *--sp = 0;          /* FIXME: ret addr should point to kernel proc-ending fn? */
        kstack_top = sp;

        r->sr = MC68K_SR_SUPERVISOR;
    }
    else
    {
        sp = (u32 *) ustack_top;
        *--sp = (u32) arg;
        *--sp = 0;          /* FIXME: ret addr should point to userspace proc-ending fn? */
        ustack_top = sp;

        r->sr = 0;
    }

    r->usp = (u32) ustack_top;
    r->pc = (u32) entry_point;

    /*
        Place a fake short-format exception stack frame at the top of the kernel stack.  This
        enables us to start the process by switching to its stack and executing an rte.
    */
    f = (mc68010_short_exc_frame_t *) ((u8 *) kstack_top - sizeof(mc68010_short_exc_frame_t));
    f->format_offset = MC68K_EXC_FMT_SHORT;
    f->pc = r->pc;
    f->sr = r->sr;

    r->a[7] = (u32) f;

    return SUCCESS;
}


/*
    mc68000_dump_status_register() - write a string describing the contents of the status register
*/
const char * mc68000_dump_status_register(ku16 sr)
{
    static char buf[16];

    /* dump format:  "Tx S M Ix XNZVC" */
    buf[0] = 'T';
    buf[1] = '0' + MC68K_SR_TRACE_LEVEL(sr);
    buf[2] = buf[4] = buf[6] = buf[9] = ' ';
    buf[3] = (sr & MC68K_SR_SUPERVISOR) ? 'S' : 'U';
    buf[5] = (sr & MC68K_SR_STACK) ? 'M' : 'I';
    buf[7] = 'I';
    buf[8] = '0' + MC68K_SR_IPL(sr);
    buf[10] = (sr & MC68K_SR_X) ? 'X' : 'x';
    buf[11] = (sr & MC68K_SR_N) ? 'N' : 'n';
    buf[12] = (sr & MC68K_SR_Z) ? 'Z' : 'z';
    buf[13] = (sr & MC68K_SR_V) ? 'V' : 'v';
    buf[14] = (sr & MC68K_SR_C) ? 'C' : 'c';
    buf[15] = '\0';

    return buf;
}


/*
    mc68000_dump_fc() - return a string describing a function code, e.g. "cpu", "ud", "sp", "fc3".
*/
const char * mc68000_dump_fc(ku16 fc)
{
    switch(fc)
    {
        case 0:     return "fc0";       /* (undefined, reserved)    */
        case 1:     return "ud";        /* user data                */
        case 2:     return "up";        /* user program             */
        case 3:     return "fc3";       /* (undefined, reserved)    */
        case 4:     return "fc4";       /* (undefined, reserved)    */
        case 5:     return "sd";        /* supervisor data          */
        case 6:     return "sp";        /* supervisor program       */
        case 7:     return "cpu";       /* CPU space                */
        default:    return "???";       /* invalid function code    */
    }
}


/*
    mc68000_dump_ssw() - return a string describing the contents of the special status word, a set
    of flags written to bus/address error exception frames.
*/
const char * mc68000_dump_ssw(ku16 ssw)
{
    static char buf[46];

    /* dump format: "RR=cpu IF=x DF=x RM=x HB=x BY=x R/W=x FC=ur " */
    sprintf(buf, "RR=%s IF=%c DF=%c RM=%c HB=%c BY=%c R/W=%c FC=%s",
            (ssw & MC68K_SSW_RR) ? "cpu" : "sw ",
            (ssw & MC68K_SSW_IF) ? '1' : '0',
            (ssw & MC68K_SSW_DF) ? '1' : '0',
            (ssw & MC68K_SSW_RM) ? '1' : '0',
            (ssw & MC68K_SSW_HB) ? '1' : '0',
            (ssw & MC68K_SSW_BY) ? '1' : '0',
            (ssw & MC68K_SSW_RW) ? 'r' : 'w',
            mc68000_dump_fc((ssw & MC68K_SSW_FC_MASK) >> MC68K_SSW_FC_SHIFT));

    return buf;
}


/*
    mc68010_dump_address_exc_frame() - dump a MC68010 exception frame (address/bus error version)
*/
void mc68010_dump_address_exc_frame(const mc68010_address_exc_frame_t * const aef)
{
#if !defined(TARGET_MC68010)
#error "This code requires a MC68010 target"
#endif
    printf("Special status word = %04x    [%s]\n"
           "Fault address       = 0x%08x\n"
           "Data output buffer  = %04x\n"
           "Data input buffer   = %04x\n"
           "Instr output buffer = %04x\n"
           "Version number      = %04x\n",
            aef->special_status_word, mc68000_dump_ssw(aef->special_status_word), aef->fault_addr,
            aef->data_output_buffer, aef->data_input_buffer, aef->instr_output_buffer,
            aef->version_number);
}


/*
    mc68000_dump_regs() - dump the contents of a regs_t struct, in human-readable form
*/
void mc68000_dump_regs(const regs_t *regs)
{
    if((addr_t) regs & 1)
    {
        puts("(regdump unavailable - stack corrupt)");
        return;
    }

    ksym_format_nearest_prev((void *) regs->pc, g_errsym, sizeof(g_errsym));

    printf("D0=%08x  D1=%08x  D2=%08x  D3=%08x\n"
           "D4=%08x  D5=%08x  D6=%08x  D7=%08x\n"
           "A0=%08x  A1=%08x  A2=%08x  A3=%08x\n"
           "A4=%08x  A5=%08x  A6=%08x  A7=%08x\n"
           "                                      USP=%08x\n"
           "PC=%08x  %s\nSR=%04x  [%s]\n",
           regs->d[0], regs->d[1], regs->d[2], regs->d[3],
           regs->d[4], regs->d[5], regs->d[6], regs->d[7],
           regs->a[0], regs->a[1], regs->a[2], regs->a[3],
           regs->a[4], regs->a[5], regs->a[6], regs->a[7], regs->usp,
           regs->pc, g_errsym, regs->sr, mc68000_dump_status_register(regs->sr));
}


/*
    mc68000_try_stack_dump() - attempt to dump nwords words, starting at the address in sp.
    Fail if sp is misaligned, or if the requested range doesn't lie within a single, valid
    memory extent.
*/
void mc68000_try_stack_dump(ku32 sp, ku32 nwords)
{
    u32 *psp;
    mem_extent_t *extent;

    if(sp & 3)
    {
        puts("(SP misaligned)");
        return;
    }

    psp = (u32 *) sp;

    if(nwords)
    {
        extent = mem_get_containing_extent(psp + nwords);
        if((extent == NULL) || !MEM_EXTENT_IS_RAM(extent)
           || (extent != mem_get_containing_extent(psp)))
        {
            puts("(SP invalid)");
            return;
        }

        dump_hex(psp, sizeof(u32), 0, nwords * sizeof(u32));
    }
}
