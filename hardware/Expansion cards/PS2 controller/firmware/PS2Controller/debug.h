/*
	debug.c - debugging functions
	
	
	(c) Stuart Wallace <stuartw@atom.net>, December 2015.
	
	
	If the constant WITH_DEBUGGING is defined, debugging functions become available.  If this macro is
	not defined, any calls to debug functions generate no code.
*/ 


#ifndef DEBUG_H_
#define DEBUG_H_

#ifdef WITH_DEBUGGING

void debug_init();
void debug_putc(const char c);
void debug_puts(const char *s);
void debug_puthexb(const char c);

#else

#define debug_init()
#define debug_putc(c)		((void) c)
#define debug_puts(s)		((void) s)
#define debug_puthexb(c)	((void) c)

#endif

#endif /* DEBUG_H_ */