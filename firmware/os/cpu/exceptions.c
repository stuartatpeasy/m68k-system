/*
	MC68010 CPU exception handler functions

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace, December 2011.
*/

#include "cpu/exceptions.h"


void __cpu_exc_install_default_handlers(void)
{
	unsigned short u;

	/* Start by pointing all exception vectors at the generic handler */
	for(u = 0; u < 256; ++u)
		CPU_EXC_VPTR_SET(u, __cpu_exc_generic);

	/* Now set specific handlers */
	V_ssp			        = CPU_EXC_VPTR(0xca5caded);				/* nonsense number */
	V_reset			        = CPU_EXC_VPTR(0xbed51de5);				/* nonsense number */
	V_bus_error		        = CPU_EXC_VPTR(__cpu_exc_bus_error);
	V_address_error	        = CPU_EXC_VPTR(__cpu_exc_address_error);

	V_level_1_autovector    = CPU_EXC_VPTR(irq_schedule);           /* scheduler IRQ handler */

	/* TODO install TRAP handlers */
	V_trap_0  = CPU_EXC_VPTR(__cpu_trap_0);
	V_trap_1  = CPU_EXC_VPTR(__cpu_trap_1);
	V_trap_2  = CPU_EXC_VPTR(__cpu_trap_2);
	V_trap_3  = CPU_EXC_VPTR(__cpu_trap_3);
	V_trap_4  = CPU_EXC_VPTR(__cpu_trap_4);
	V_trap_5  = CPU_EXC_VPTR(__cpu_trap_5);
	V_trap_6  = CPU_EXC_VPTR(__cpu_trap_6);
	V_trap_7  = CPU_EXC_VPTR(__cpu_trap_7);
	V_trap_8  = CPU_EXC_VPTR(__cpu_trap_8);
	V_trap_9  = CPU_EXC_VPTR(__cpu_trap_9);
	V_trap_10 = CPU_EXC_VPTR(__cpu_trap_10);
	V_trap_11 = CPU_EXC_VPTR(__cpu_trap_11);
	V_trap_12 = CPU_EXC_VPTR(__cpu_trap_12);
	V_trap_13 = CPU_EXC_VPTR(__cpu_trap_13);
	V_trap_14 = CPU_EXC_VPTR(__cpu_trap_14);
	V_trap_15 = CPU_EXC_VPTR(__cpu_trap_15);
}


void __cpu_exc_bus_error(int dummy, const struct __mc68010_address_exc_frame f)
{
    cpu_disable_interrupts();
	puts("\nException: Bus error");
	__cpu_dump_address_exc_frame(&f);
	__cpu_halt();
}


void __cpu_exc_address_error(int dummy, const struct __mc68010_address_exc_frame f)
{
    cpu_disable_interrupts();
	puts("\nException: Address error");
	__cpu_dump_address_exc_frame(&f);
	__cpu_halt();
}


void __cpu_exc_generic(const struct __mc68010_exc_frame f)
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

	__cpu_dump_exc_frame(&f);
	__cpu_halt();
}


void __cpu_trap_0(void)
{
	puts("TRAP 0");
}


void __cpu_trap_1(void)
{
	puts("TRAP 1");
}


void __cpu_trap_2(void)
{
	puts("TRAP 2");
}


void __cpu_trap_3(void)
{
	puts("TRAP 3");
}


void __cpu_trap_4(void)
{
	puts("TRAP 4");
}


void __cpu_trap_5(void)
{
	puts("TRAP 5");
}


void __cpu_trap_6(void)
{
	puts("TRAP 6");
}


void __cpu_trap_7(void)
{
	puts("TRAP 7");
}


void __cpu_trap_8(void)
{
	puts("TRAP 8");
}


void __cpu_trap_9(void)
{
	puts("TRAP 9");
}


void __cpu_trap_10(void)
{
	puts("TRAP 10");
}


void __cpu_trap_11(void)
{
	puts("TRAP 11");
}


void __cpu_trap_12(void)
{
	puts("TRAP 12");
}


void __cpu_trap_13(void)
{
	puts("TRAP 13");
}


void __cpu_trap_14(void)
{
	puts("TRAP 14");
}


void __cpu_trap_15(void)
{
	puts("TRAP 15");
}
