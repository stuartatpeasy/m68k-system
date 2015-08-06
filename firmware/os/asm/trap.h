#ifndef ASM_TRAP_H_INC
#define ASM_TRAP_H_INC
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
#define trap(x)	do					    		\
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
#define trapv()	do					    		\
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
#define trap0()	    trap(0)
#define trap1()	    trap(1)
#define trap2()	    trap(2)
#define trap3()	    trap(3)
#define trap4()	    trap(4)
#define trap5()	    trap(5)
#define trap6()	    trap(6)
#define trap7()	    trap(7)
#define trap8()	    trap(8)
#define trap9()	    trap(9)
#define trap10()	trap(10)
#define trap11()	trap(11)
#define trap12()	trap(12)
#define trap13()	trap(13)
#define trap14()	trap(14)
#define trap15()	trap(15)

#endif

