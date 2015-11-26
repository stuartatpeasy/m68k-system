/*
	ayumos.h: definitions needed to build userland apps for ayumos

	Stuart Wallace, August 2015.
*/


/* System call numbers */
/* TODO: read these from the os/ dir, instead of having a copy of them here */
#define SYS_exit    	        0
#define SYS_console_putchar     1
#define SYS_console_getchar	    2
#define SYS_leds                3
#define SYS_yield				4

#define sys_exit(arg1)					syscall1(SYS_exit, (arg1))
#define sys_console_getchar()			syscall0(SYS_console_getchar)
#define sys_console_putchar(arg1)		syscall1(SYS_console_putchar, (arg1))
#define sys_leds(arg1)					syscall1(SYS_leds, (arg1))
#define sys_yield()						syscall0(SYS_yield)

int syscall0(const unsigned int func);
int syscall1(const unsigned int func, const unsigned int arg1);
int syscall2(const unsigned int func, const unsigned int arg1, const unsigned int arg2);
int syscall3(const unsigned int func, const unsigned int arg1, const unsigned int arg2, const unsigned int arg3);

void _start();

extern int main();


