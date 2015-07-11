/*
	MC68010 CPU utility functions

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace, December 2011.
*/

#include "cpu/utilities.h"
#include "kutil/kutil.h"


inline void cpu_halt(void)
{
	puts("\nSystem halted.");

    /* the arg to "stop" causes the CPU to stay in supervisor mode,
       and sets the interrupt mask to 7 */
	__asm__ volatile("stop #0x2700" : );
	while(1) ;
}


/*
	Write a string describing the contents of the status register
*/
const char * const cpu_dump_status_register(ku16 sr)
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
void cpu_dump_exc_frame(const struct mc68010_exc_frame * const f)
{
	printf("Status register     = %04x [%s]\n"
		   "Program counter     = 0x%08x\n"
		/* "Frame format        = %x\n" */
		   "Vector offset       = %04x\n",
			f->sr, cpu_dump_status_register(f->sr), f->pc,
			/*(f->vector_offset >> 12) & 0xf, */ f->vector_offset & 0xfff);
}


/*
	Dump a MC68010 exception frame (address/bus error version)

	TODO: remove the dependency on printf()
*/
void cpu_dump_address_exc_frame(const struct mc68010_address_exc_frame * const f)
{
	printf("Status register     = %04x [%s]\n"
		   "Program counter     = 0x%08x\n"
		/* "Frame format        = %x\n" */
		   "Vector offset       = %04x\n"
		   "Special status word = %04x\n"
		   "Fault address       = 0x%08x\n"
		   "Data output buffer  = %04x\n"
		   "Data input buffer   = %04x\n"
		   "Instr output buffer = %04x\n"
		   "Version number      = %04x\n",
			f->sr, cpu_dump_status_register(f->sr), f->pc,
			/* (f->vector_offset >> 12) & 0xf, */ f->vector_offset & 0xfff, f->special_status_word,
			f->fault_addr, f->data_output_buffer, f->data_input_buffer, f->instr_output_buffer,
			f->version_number);
}

