#ifndef INCLUDE_ERROR_H_INC
#define INCLUDE_ERROR_H_INC
/*
	error.h: kernel internal error codes

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace, August 2015.
*/

/*
    Kernel-specific error codes extend the "standard" error codes provided in errno.h.  Kernel
    functions return the constant SUCCESS (defined as 0 in include/defs.h) on success, or an
    appropriate error code.  No counterpart to the "errno" global var exists in the kernel.

*/
#include "klibc/errno.h"

#define KERNEL_MIN_ERRNO    (500)    /* First error num reserved for custom error messages */

#define EUNKNOWN            (500)   /* Unknown error                                    */
#define EMEDIA              (501)   /* Media error                                      */
#define EMEDIACHANGED       (502)   /* Media changed                                    */
#define EDATA               (503)   /* Data error                                       */
#define ECKSUM              (504)   /* Checksum error                                   */
#define EBADSBLK            (505)   /* Bad superblock                                   */

#endif
