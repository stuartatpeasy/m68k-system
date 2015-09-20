#ifndef MONITOR_DISASM_H_INC
#define MONITOR_DISASM_H_INC
/*
	MC68000/68010 disassembler

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace, 2011.
*/

#include <stdio.h>
#include <string.h>
#include "assert.h"
#include "kutil/kutil.h"

#define BEW_DISPLACEMENT(x)		((char) ((x) & 0xff))
#define BEW_REGISTER(x)			(((x) & 0x7000) >> 12)
#define BEW_SCALE(x)			(((x) & 0x0600) >> 9)
#define BEW_DA(x)				(((x) & 0x8000) ? 'a' : 'd')

#define TEST(x, y)              (((x) & (1 << (y))) ? 1 : 0)

/*
	host-to-platform (HTOP_*) macros

	If the host is little-endian, this module must be compiled with -DHOST_LITTLEENDIAN -
	the HTOP_* macros will then swap the byte order of short ints and ints.  Otherwise
	the code in this module will assume a big-endian host (e.g. the m68k hardware).
*/
#ifdef HOST_LITTLEENDIAN
#define HTOP_SHORT(x)			((((x) & 0xff) << 8) | (((x) >> 8) & 0xff))
#define HTOP_INT(x)				(((x) << 24) | (((x) & 0xff00) << 8) | (((x) & 0xff0000) >> 8) | \
                                    ((x) >> 24))
#else
#define HTOP_SHORT(x)			(x)
#define HTOP_INT(x)				(x)
#endif

/*
	addressing modes used in effective addresses
*/
#define EAMODE_DN				(0)	/* data register direct									 */
#define EAMODE_AN				(1)	/* address register direct								 */
#define EAMODE_AN_IND			(2)	/* address register indirect							 */
#define EAMODE_AN_IND_POSTINC	(3)	/* address register indirect with postincrement			 */
#define EAMODE_AN_IND_PREDEC	(4)	/* address register indirect with predecrement			 */
#define EAMODE_AN_DISP			(5)	/* address register indirect with displacement			 */
#define EAMODE_AN_DISP_INDEX	(6)	/* address register indirect with displacement and index */
#define EAMODE_IMM				(7)	/* immediate / absolute word/long / PC+disp / PC+d+index */

/*
	addressing sub-modes used to identify the type of "immediate" address in an effective address
*/
#define EAMODE_IMM_ABSW				(0)		/* absolute word								*/
#define EAMODE_IMM_ABSL				(1)		/* absolute long								*/
#define EAMODE_IMM_PC_DISP			(2)		/* program counter indirect with displacement	*/
#define EAMODE_IMM_PC_DISP_INDEX	(3)		/* program counter indirect with disp + index	*/
#define EAMODE_IMM_IMM				(4)		/* immediate value								*/

typedef enum ea_size
{
	ea_unsized = 0,
	ea_byte = 'b',
	ea_word = 'w',
	ea_long = 'l',
	ea_sr = 's'			/* flag used to indicate an operation on the status register */
} ea_size_t;

int disassemble(unsigned short **p, char *str);

/* TODO: make these static */
char *ea(char *str, unsigned char mode, unsigned char reg, unsigned short **p, const ea_size_t sz);
const char * const cond(unsigned char code);
void movem_regs(char *str, unsigned short regs, char mode);

#endif
