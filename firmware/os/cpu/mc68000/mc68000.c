/*
    Implementations of MC68000-specific functions

    Part of ayumos


    (c) Stuart Wallace <stuartw@atom.net>, September 2015.
*/

#include <cpu/mc68000/mc68000.h>
#include <klibc/stdio.h>


/*
    Initialise the MC68000 vector table to some safe defaults.  This should ensure that any IRQs
    are handled correctly, by being ignored, or reported and ignored, or being reported and
    followed by a CPU halt.
*/
void cpu_init_interrupt_handlers(void)
{
	u16 u;

    /* Initialise the generic IRQ handler's jump table */
    for(u = 0; u <= CPU_MAX_IRQL; ++u)
        cpu_set_interrupt_handler(u, NULL, mc68000_exc_generic);

	/* Point all exception vectors at the generic IRQ handler code initially */
	for(u = 0; u <= CPU_MAX_IRQL; ++u)
        CPU_EXC_VPTR_SET(u, irq_handler);

	/* Now set specific handlers */
	CPU_EXC_VPTR_SET(V_ssp,             0xca5caded);                /* nonsense number */
	CPU_EXC_VPTR_SET(V_reset,           0xbed51de5);                /* nonsense number */

	cpu_set_interrupt_handler(V_bus_error, NULL, mc68000_exc_bus_error);
	cpu_set_interrupt_handler(V_address_error, NULL, mc68000_exc_address_error);

	/* TODO install TRAP handlers */
	cpu_set_interrupt_handler(V_trap_0, NULL, mc68000_trap_0_handler);
	cpu_set_interrupt_handler(V_trap_1, NULL, mc68000_trap_1_handler);
	cpu_set_interrupt_handler(V_trap_2, NULL, mc68000_trap_2_handler);
	cpu_set_interrupt_handler(V_trap_3, NULL, mc68000_trap_3_handler);
	cpu_set_interrupt_handler(V_trap_4, NULL, mc68000_trap_4_handler);
	cpu_set_interrupt_handler(V_trap_5, NULL, mc68000_trap_5_handler);
	cpu_set_interrupt_handler(V_trap_6, NULL, mc68000_trap_6_handler);
	cpu_set_interrupt_handler(V_trap_7, NULL, mc68000_trap_7_handler);
	cpu_set_interrupt_handler(V_trap_8, NULL, mc68000_trap_8_handler);
	cpu_set_interrupt_handler(V_trap_9, NULL, mc68000_trap_9_handler);
	cpu_set_interrupt_handler(V_trap_10, NULL, mc68000_trap_10_handler);
	cpu_set_interrupt_handler(V_trap_11, NULL, mc68000_trap_11_handler);
	cpu_set_interrupt_handler(V_trap_12, NULL, mc68000_trap_12_handler);
	cpu_set_interrupt_handler(V_trap_13, NULL, mc68000_trap_13_handler);
	cpu_set_interrupt_handler(V_trap_14, NULL, mc68000_trap_14_handler);
	cpu_set_interrupt_handler(V_trap_15, NULL, mc68000_trap_15_handler);
}


void mc68000_exc_bus_error(ku32 irql, void *data, const regs_t regs)
{
    cpu_disable_interrupts();
	puts("\nException: Bus error");

	mc68010_dump_address_exc_frame(irql, &regs);
	putchar('\n');
    mc68000_dump_regs(&regs);

	puts("\nSystem halted.");
	cpu_halt();
}


void mc68000_exc_address_error(ku32 irql, void *data, const regs_t regs)
{
    cpu_disable_interrupts();
	puts("\nException: Address error");

	mc68010_dump_address_exc_frame(irql, &regs);
	putchar('\n');
    mc68000_dump_regs(&regs);

	puts("\nSystem halted.");
	cpu_halt();
}


void mc68000_exc_generic(ku32 irql, void *data, const regs_t regs)
{
    cpu_disable_interrupts();
	printf("\nUnhandled exception: ");
	if((irql >= 25) && (irql <= 31))
		printf("Level %d interrupt autoirqltor\n", irql - 24);
	else if((irql >= 32) && (irql <= 47))
		printf("Trap #%d\n", irql - 32);
	else if(irql >= 64)
		printf("User-defined vector %d\n", irql - 64);
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
			case 54: msg = "FP signaling NaN";						break;
			case 55: msg = "FP unimplemented data type";			break;
			case 56: msg = "MMU configuration error";				break;
			case 57: msg = "MMU illegal operation";					break;
			case 58: msg = "MMU access level violation";			break;
		}

		if(msg)
			puts(msg);
		else
			printf("Unknown #%d\n", irql);
	}

	mc68010_dump_exc_frame(irql, &regs);
	putchar('\n');
    mc68000_dump_regs(&regs);

	puts("\nSystem halted.");
	cpu_halt();
}


void mc68000_trap_0_handler(ku32 irql, void *data, const regs_t regs)
{
	puts("TRAP 0");
}


void mc68000_trap_1_handler(ku32 irql, void *data, const regs_t regs)
{
	puts("TRAP 1");
}


void mc68000_trap_2_handler(ku32 irql, void *data, const regs_t regs)
{
	puts("TRAP 2");
}


void mc68000_trap_3_handler(ku32 irql, void *data, const regs_t regs)
{
	puts("TRAP 3");
}


void mc68000_trap_4_handler(ku32 irql, void *data, const regs_t regs)
{
	puts("TRAP 4");
}


void mc68000_trap_5_handler(ku32 irql, void *data, const regs_t regs)
{
	puts("TRAP 5");
}


void mc68000_trap_6_handler(ku32 irql, void *data, const regs_t regs)
{
	puts("TRAP 6");
}


void mc68000_trap_7_handler(ku32 irql, void *data, const regs_t regs)
{
	puts("TRAP 7");
}


void mc68000_trap_8_handler(ku32 irql, void *data, const regs_t regs)
{
	puts("TRAP 8");
}


void mc68000_trap_9_handler(ku32 irql, void *data, const regs_t regs)
{
	puts("TRAP 9");
}


void mc68000_trap_10_handler(ku32 irql, void *data, const regs_t regs)
{
	puts("TRAP 10");
}


void mc68000_trap_11_handler(ku32 irql, void *data, const regs_t regs)
{
	puts("TRAP 11");
}


void mc68000_trap_12_handler(ku32 irql, void *data, const regs_t regs)
{
	puts("TRAP 12");
}


void mc68000_trap_13_handler(ku32 irql, void *data, const regs_t regs)
{
	puts("TRAP 13");
}


void mc68000_trap_14_handler(ku32 irql, void *data, const regs_t regs)
{
	puts("TRAP 14");
}


void mc68000_trap_15_handler(ku32 irql, void *data, const regs_t regs)
{
	puts("TRAP 15");
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
	Write a string describing the contents of the status register
*/
const char * const mc68000_dump_status_register(ku16 sr)
{
	static char buf[16];

	/* dump format:  Tx S M Ix XNZVC */
	/* buf must point to a >= 16-byte buffer */
	buf[0] = 'T';
	buf[1] = '0' + ((sr >> 14) & 3);
	buf[2] = buf[4] = buf[6] = buf[9] = ' ';
	buf[3] = (sr & 0x2000) ? 'S' : 'U';
	buf[5] = (sr & 0x1000) ? 'M' : 'I';
	buf[7] = 'I';
	buf[8] = '0' + ((sr >> 8) & 7);
	buf[10] = (sr & 0x10) ? 'X' : 'x';
	buf[11] = (sr & 0x08) ? 'N' : 'n';
	buf[12] = (sr & 0x04) ? 'Z' : 'z';
	buf[13] = (sr & 0x02) ? 'V' : 'v';
	buf[14] = (sr & 0x01) ? 'C' : 'c';
	buf[15] = '\0';

	return buf;
}


/*
	Dump a MC68010 exception frame (short version)

	TODO: remove the dependency on printf()
*/
void mc68010_dump_exc_frame(ku32 irql, const regs_t * const regs)
{
#if !defined(TARGET_MC68010)
#error "This code requires a MC68010 target"
#endif
    char sym[128];

    ksym_format_nearest_prev((void *) regs->pc, sym, sizeof(sym));

	printf("Status register     = %04x [%s]\n"
		   "Program counter     = 0x%08x    %s\n"
		   "Vector number       = %u\n\n",
			regs->sr, mc68000_dump_status_register(regs->sr), regs->pc, sym, irql);
}


/*
	Dump a MC68010 exception frame (address/bus error version)

	TODO: remove the dependency on printf()
*/
void mc68010_dump_address_exc_frame(ku32 irql, const regs_t * const regs)
{
#if !defined(TARGET_MC68010)
#error "This code requires a MC68010 target"
#endif
    const struct mc68010_address_exc_frame * const f =
        (const struct mc68010_address_exc_frame * const) &(regs[1]);

    char sym[128];

    ksym_format_nearest_prev((void *) regs->pc, sym, sizeof(sym));

	printf("Status register     = %04x [%s]\n"
		   "Program counter     = 0x%08x    %s\n"
		/* "Frame format        = %x\n" */
		   "Vector number       = %u\n"
		   "Special status word = %04x\n"
		   "Fault address       = 0x%08x\n"
		   "Data output buffer  = %04x\n"
		   "Data input buffer   = %04x\n"
		   "Instr output buffer = %04x\n"
		   "Version number      = %04x\n",
			regs->sr, mc68000_dump_status_register(regs->sr), regs->pc, sym, irql,
            f->special_status_word, f->fault_addr, f->data_output_buffer, f->data_input_buffer,
            f->instr_output_buffer,	f->version_number);
}


/*
    mc68000_dump_regs() - dump the contents of a regs_t struct, in human-readable form
*/
void mc68000_dump_regs(const regs_t *regs)
{
    printf("D0=%08x  D1=%08x  D2=%08x  D3=%08x\n"
           "D4=%08x  D5=%08x  D6=%08x  D7=%08x\n"
           "A0=%08x  A1=%08x  A2=%08x  A3=%08x\n"
           "A4=%08x  A5=%08x  A6=%08x  SP=%08x\n\n"
           "PC=%08x  SR=%04x  [%s]\n",
           regs->d[0], regs->d[1], regs->d[2], regs->d[3],
           regs->d[4], regs->d[5], regs->d[6], regs->d[7],
           regs->a[0], regs->a[1], regs->a[2], regs->a[3],
           regs->a[4], regs->a[5], regs->a[6], regs->a[7],
           regs->pc, regs->sr, mc68000_dump_status_register(regs->sr));
}
