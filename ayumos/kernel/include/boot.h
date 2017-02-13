#ifndef KERNEL_INCLUDE_BOOT_H_INC
#define KERNEL_INCLUDE_BOOT_H_INC
/*
    boot.h: declarations of functions useful during boot

    Part of the as-yet-unnamed MC68010 operating system


    (c) Stuart Wallace, 17th October 2015.
*/

#include <kernel/include/defs.h>
#include <kernel/include/types.h>


/* Early-boot failure codes: these are presented as a number of flashes of the red LED */
#define BOOT_FAIL_PLATFORM_INIT         (1)
#define BOOT_FAIL_MEMORY_DETECT         (2)
#define BOOT_FAIL_DEVICE_INIT           (3)
#define BOOT_FAIL_EARLY_CONSOLE_INIT    (4)
#define BOOT_FAIL_DEVICE_ENUMERATE      (5)
#define BOOT_FAIL_CONSOLE_INIT          (6)


void boot_early_fail(ku32 code);
void boot_list_mass_storage();
void boot_list_partitions();

#endif
