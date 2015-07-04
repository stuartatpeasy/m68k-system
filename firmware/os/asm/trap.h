#ifndef __ASM_TRAP_H__
#define __ASM_TRAP_H__
/*
	TRAP machine instruction "intrinsic"

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace, December 2011.
*/

/*
	Generic TRAP macro

	NOTE: if x > 15, the assembler will generate a warning and use the value 0 instead.  It might
	therefore be better to only offer macros for specific TRAP levels, as below.
*/
#define __trap(x)	do							\
					{							\
						__asm__ __volatile__	\
						(						\
							"trap %w0"			\
							: 					\
							: "n" (x)			\
						);						\
					} while(0);					\

/*
	TRAPV macro
*/
#define __trapv()	do							\
					{							\
						__asm__ __volatile__	\
						(						\
							"trapv"				\
							: :					\
						);						\
					} while(0);					\

/*
	Level-specific TRAP macros
*/
#define __trap0()	__trap(0)
#define __trap1()	__trap(1)
#define __trap2()	__trap(2)
#define __trap3()	__trap(3)
#define __trap4()	__trap(4)
#define __trap5()	__trap(5)
#define __trap6()	__trap(6)
#define __trap7()	__trap(7)
#define __trap8()	__trap(8)
#define __trap9()	__trap(9)
#define __trap10()	__trap(10)
#define __trap11()	__trap(11)
#define __trap12()	__trap(12)
#define __trap13()	__trap(13)
#define __trap14()	__trap(14)
#define __trap15()	__trap(15)

#endif

