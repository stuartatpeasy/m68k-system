/*
	MC68010 CPU exception handler functions

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace, December 2011.
*/

#include "cpu/exceptions.h"


/*
	Install the default handlers for every exception.  Default behaviour for every exception is to
	report the exception and dump its stack frame, then halt the system.
*/
void cpu_exc_install_default_handlers(void)
{
	unsigned short u;

	/* Start by pointing all exception vectors at the generic handler */
	for(u = 0; u < 256; ++u)
        CPU_EXC_VPTR_SET(u, cpu_exc_generic);

	/* Now set specific handlers */
	CPU_EXC_VPTR_SET(V_ssp,             0xca5caded);                /* nonsense number */
	CPU_EXC_VPTR_SET(V_reset,           0xbed51de5);                /* nonsense number */
	CPU_EXC_VPTR_SET(V_bus_error,       cpu_exc_bus_error);
	CPU_EXC_VPTR_SET(V_address_error,   cpu_exc_address_error);

	/* TODO install TRAP handlers */
	cpu_set_interrupt_handler(V_trap_0, NULL, cpu_trap_0_handler);
	cpu_set_interrupt_handler(V_trap_1, NULL, cpu_trap_1_handler);
	cpu_set_interrupt_handler(V_trap_2, NULL, cpu_trap_2_handler);
	cpu_set_interrupt_handler(V_trap_3, NULL, cpu_trap_3_handler);
	cpu_set_interrupt_handler(V_trap_4, NULL, cpu_trap_4_handler);
	cpu_set_interrupt_handler(V_trap_5, NULL, cpu_trap_5_handler);
	cpu_set_interrupt_handler(V_trap_6, NULL, cpu_trap_6_handler);
	cpu_set_interrupt_handler(V_trap_7, NULL, cpu_trap_7_handler);
	cpu_set_interrupt_handler(V_trap_8, NULL, cpu_trap_8_handler);
	cpu_set_interrupt_handler(V_trap_9, NULL, cpu_trap_9_handler);
	cpu_set_interrupt_handler(V_trap_10, NULL, cpu_trap_10_handler);
	cpu_set_interrupt_handler(V_trap_11, NULL, cpu_trap_11_handler);
	cpu_set_interrupt_handler(V_trap_12, NULL, cpu_trap_12_handler);
	cpu_set_interrupt_handler(V_trap_13, NULL, cpu_trap_13_handler);
	cpu_set_interrupt_handler(V_trap_14, NULL, cpu_trap_14_handler);
	cpu_set_interrupt_handler(V_trap_15, NULL, cpu_trap_15_handler);
}


void cpu_exc_bus_error(int dummy, const struct mc68010_address_exc_frame f)
{
    cpu_disable_interrupts();
	puts("\nException: Bus error");
	cpu_dump_address_exc_frame(&f);
	cpu_halt();
}


void cpu_exc_address_error(int dummy, const struct mc68010_address_exc_frame f)
{
    cpu_disable_interrupts();
	puts("\nException: Address error");
	cpu_dump_address_exc_frame(&f);
	cpu_halt();
}


void cpu_exc_generic(const struct mc68010_exc_frame f)
{
	const unsigned char vec = ((f.vector_offset & 0xfff) >> 2);

    cpu_disable_interrupts();
	printf("\nUnhandled exception: ");
	if((vec >= 25) && (vec <= 31))
		printf("Level %d interrupt autovector\n", vec - 24);
	else if((vec >= 32) && (vec <= 47))
		printf("Trap #%d\n", vec - 32);
	else if(vec >= 64)
		printf("User-defined vector %d\n", vec - 64);
	else
	{
		const char *msg = NULL;
		switch(vec)
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
			printf("Unknown #%d\n", vec);
	}

	cpu_dump_exc_frame(&f);
	cpu_halt();
}


void cpu_trap_0_handler(u16 irql, void *data, const struct regs * regs)
{
	puts("TRAP 0");
}


void cpu_trap_1_handler(u16 irql, void *data, const struct regs * regs)
{
	puts("TRAP 1");
}


void cpu_trap_2_handler(u16 irql, void *data, const struct regs * regs)
{
	puts("TRAP 2");
}


void cpu_trap_3_handler(u16 irql, void *data, const struct regs * regs)
{
	puts("TRAP 3");
}


void cpu_trap_4_handler(u16 irql, void *data, const struct regs * regs)
{
	puts("TRAP 4");
}


void cpu_trap_5_handler(u16 irql, void *data, const struct regs * regs)
{
	puts("TRAP 5");
}


void cpu_trap_6_handler(u16 irql, void *data, const struct regs * regs)
{
	puts("TRAP 6");
}


void cpu_trap_7_handler(u16 irql, void *data, const struct regs * regs)
{
	puts("TRAP 7");
}


void cpu_trap_8_handler(u16 irql, void *data, const struct regs * regs)
{
	puts("TRAP 8");
}


void cpu_trap_9_handler(u16 irql, void *data, const struct regs * regs)
{
	puts("TRAP 9");
}


void cpu_trap_10_handler(u16 irql, void *data, const struct regs * regs)
{
	puts("TRAP 10");
}


void cpu_trap_11_handler(u16 irql, void *data, const struct regs * regs)
{
	puts("TRAP 11");
}


void cpu_trap_12_handler(u16 irql, void *data, const struct regs * regs)
{
	puts("TRAP 12");
}


void cpu_trap_13_handler(u16 irql, void *data, const struct regs * regs)
{
	puts("TRAP 13");
}


void cpu_trap_14_handler(u16 irql, void *data, const struct regs * regs)
{
	puts("TRAP 14");
}


void cpu_trap_15_handler(u16 irql, void *data, const struct regs * regs)
{
	puts("TRAP 15");
}
