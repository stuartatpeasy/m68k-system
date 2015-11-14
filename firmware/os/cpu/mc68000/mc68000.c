/*
    Implementations of MC68000-specific functions

    Part of ayumos


    (c) Stuart Wallace <stuartw@atom.net>, September 2015.
*/

#include <cpu/mc68000/mc68000.h>
#include <kernel/process.h>
#include <klibc/stdio.h>


/*
    Initialise the MC68000 vector table to some safe defaults.  This should ensure that any IRQs
    are handled correctly, by being ignored, or reported and ignored, or being reported and
    followed by a CPU halt.
*/
void cpu_irq_init_arch_specific(void)
{
	u16 u;

	/* Point all exception vectors at the generic IRQ handler code initially */
	for(u = 0; u <= CPU_MAX_IRQL; ++u)
        CPU_EXC_VPTR_SET(u, irq_router_full);

	/* Now set specific handlers */
	CPU_EXC_VPTR_SET(V_ssp,             0xca5caded);                /* nonsense number */
	CPU_EXC_VPTR_SET(V_reset,           0xbed51de5);                /* nonsense number */

    /* System calls use TRAP #0 */
	CPU_EXC_VPTR_SET(V_trap_0, syscall_dispatcher);
}


/*
    cpu_default_irq_handler() - handler for interrupts not otherwise handled.  Dumps state and halts
    the CPU.
*/
void cpu_default_irq_handler(ku32 irql, void *data)
{
    const regs_t * const regs = &proc_current()->regs;
    UNUSED(data);

	printf("\nUnhandled exception in process %d: ", proc_get_pid());
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
			case 2:  msg = "Bus error";								break;
			case 3:  msg = "Address error";							break;
			case 4:  msg = "Illegal instruction";					break;
			case 5:  msg = "Integer divide by zero";				break;
			case 6:  msg = "CHK/CHK2 instruction";					break;
			case 7:  msg = "FTRAPcc/TRAPcc/TRAPV instruction";		break;
			case 8:  msg = "Privilege violation";					break;
			case 9:  msg = "Trace";									break;
			case 10: msg = "Line 1010 emulator";					break;
			case 11: msg = "Line 1111 emulator";					break;
			case 13: msg = "Coprocessor protocol violation";		break;
			case 14: msg = "Format error";							break;
			case 15: msg = "Unitialised interrupt";					break;
			case 24: msg = "Spurious interrupt";					break;
			case 48: msg = "FP branch/set on unordered condition";	break;
			case 49: msg = "FP inexact result";						break;
			case 50: msg = "FP divide by zero";						break;
			case 51: msg = "FP underflow";							break;
			case 52: msg = "FP operand error";						break;
			case 53: msg = "FP overflow";							break;
			case 54: msg = "FP signalling NaN";						break;
			case 55: msg = "FP unimplemented data type";			break;
			case 56: msg = "MMU configuration error";				break;
			case 57: msg = "MMU illegal operation";					break;
			case 58: msg = "MMU access level violation";			break;
			default: msg = "Unknown interrupt";                     break;
		}

        printf("%s (vector %d)\n", msg, irql);
	}

    if((irql == V_bus_error) || (irql == V_address_error))
    {
        putchar('\n');
        mc68010_dump_address_exc_frame(irql, regs);
    }

    putchar('\n');
    mc68000_dump_regs(regs);

	puts("\nSystem halted.");
	cpu_halt();
}


/*
    cpu_reset() - reset the MC68000.
*/
void cpu_reset(void)
{
    asm volatile
    (
        "reset                          \n"
        "lea.l      0x00f00000, %%a0    \n"     /* FIXME - board-specific ROM location */
        "move.l     %%a0@, %%a7         \n"
        "addq.l     #4, %%a0\n          \n"
        "move.l     %%a0@, %%a0         \n"
        "jmp        %%a0@               \n"
        :
        :
    );

    /* Won't return */
    while(1) ;
}


/*
    cpu_halt() - stop processing.
*/
void cpu_halt(void)
{
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
void cpu_swi()
{
    asm volatile
    (
        "trap #15                       \n"
        :
        :
    );
}


/*
    cpu_tas() - atomically test and set a byte-sized memory location to 1, returning the previous
    contents of the location.
*/
u8 cpu_tas(u8 *addr)
{
    register u32 ret = 0;

    asm volatile
    (
        "      moveq    #0, %0          \n"
        "      tas      %1              \n"
        "      beq      L_%=            \n"
        "      moveq    #1, %0          \n"
        "L_%=:                          \n"
        : "=&r" (ret)
        : "m" (*addr)
        : "cc"
    );

    return ret;
}


/*
    cpu_proc_init() - perform architecture-specific register initialisation before a new process
    starts.
*/
s32 cpu_proc_init(regs_t *r, void *entry_point, void *ustack_top, void *kstack_top, ku32 flags)
{
    mc68010_short_exc_frame_t *f;

    /* MC680x0 requires that the PC and SP are aligned to an even-numbered address */
    if(((u32) entry_point & 1) || (((u32) kstack_top) & 1) || (((u32) ustack_top) & 1))
        return EINVAL;

    if(flags & PROC_TYPE_KERNEL)
        r->sr = MC68K_SR_SUPERVISOR;
    else
        r->sr = 0;

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

	/* dump format:  Tx S M Ix XNZVC */
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
	mc68010_dump_address_exc_frame() - dump a MC68010 exception frame (address/bus error version)
*/
void mc68010_dump_address_exc_frame(ku32 irql, const regs_t * const regs)
{
#if !defined(TARGET_MC68010)
#error "This code requires a MC68010 target"
#endif
    UNUSED(irql);

    const struct mc68010_address_exc_frame * const f =
        (const struct mc68010_address_exc_frame * const) &(regs[1]);

	printf("Special status word = %04x\n"
		   "Fault address       = 0x%08x\n"
		   "Data output buffer  = %04x\n"
		   "Data input buffer   = %04x\n"
		   "Instr output buffer = %04x\n"
		   "Version number      = %04x\n",
			f->special_status_word, f->fault_addr, f->data_output_buffer,
			f->data_input_buffer, f->instr_output_buffer,	f->version_number);
}


/*
    mc68000_dump_regs() - dump the contents of a regs_t struct, in human-readable form
*/
void mc68000_dump_regs(const regs_t *regs)
{
    char sym[128];

    ksym_format_nearest_prev((void *) regs->pc, sym, sizeof(sym));

    printf("D0=%08x  D1=%08x  D2=%08x  D3=%08x\n"
           "D4=%08x  D5=%08x  D6=%08x  D7=%08x\n"
           "A0=%08x  A1=%08x  A2=%08x  A3=%08x\n"
           "A4=%08x  A5=%08x  A6=%08x  A7=%08x\n"
           "                                      USP=%08x\n\n"
           "PC=%08x  %s\nSR=%04x  [%s]\n",
           regs->d[0], regs->d[1], regs->d[2], regs->d[3],
           regs->d[4], regs->d[5], regs->d[6], regs->d[7],
           regs->a[0], regs->a[1], regs->a[2], regs->a[3],
           regs->a[4], regs->a[5], regs->a[6], regs->a[7], regs->usp,
           regs->pc, sym, regs->sr, mc68000_dump_status_register(regs->sr));
}
