#ifndef __TACHE_H__
#define __TACHE_H__

/* syscall taking a single pointer to void and returning int */
/*

not really working

#define syscall_i_p(n, p)		\
					(__extension__ ({	\
						register unsigned long __ret, __n(n), __p(p);	\
						__asm__ __volatile__	\
						(	\
							"move.l %1, %%sp@-\n"	\
							"move.l %0, %%d0\n"		\
							"trap #1\n"	\
							: "=r" (__ret)	\
							: "d" (__n), "d" (__p)	\
							: "cc"	\
						);	\
						__ret;	\
					}))

#define os_free(const void *ptr) syscall_i_p(1, ptr)
*/


void *os_malloc(const unsigned int size);
inline void os_free(const void *ptr);

#endif

