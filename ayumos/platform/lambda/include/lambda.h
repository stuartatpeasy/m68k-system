#ifndef PLATFORM_LAMBDA_REV0_LAMBDA_H_INC
#define PLATFORM_LAMBDA_REV0_LAMBDA_H_INC
/*
    ayumos port for the "lambda" (MC68010) motherboard

    Stuart Wallace, September 2015
*/

#include <driver/mc68681.h>
#include <kernel/include/device/device.h>

/* Verify build configuration */
#ifndef WITH_MASS_STORAGE
#error This port requires kernel mass-storage support (build option WITH_MASS_STORAGE)
#endif

#ifndef WITH_RTC
#error This port requires kernel real-time clock support (build option WITH_RTC)
#endif

#ifndef WITH_DRV_MST_ATA
#error This port requires the ATA driver (build option WITH_DRV_MST_ATA)
#endif

#ifndef WITH_DRV_SER_MC68681
#error This port requires the MC68681 driver (build option WITH_DRV_SER_MC68681)
#endif

#ifndef WITH_DRV_RTC_DS17485
#error This port requires the DS17485 driver (build option WITH_DRV_RTC_DS17485)
#endif
/* End verification of build configuration */

/* Firmware ROM memory start address and length */
#define LAMBDA_ROM_START        (0xf00000)      /* Start of OS ROM in memory map    */
#define LAMBDA_ROM_LENGTH       (0x100000)      /* Size of OS ROM                   */


/* See kernel/platform.h for an explanation of this #define */
#define PLATFORM_QUANTUM_USES_MACROS

/*
    plat_start_quantum() - start the quantum timer, i.e. begin a new process time-slice.

    Note: this function will be called in interrupt context.

    This function is inlined, as it is used during context-switching and is therefore
    performance-critical.
*/
#define plat_start_quantum()                                                    \
{                                                                               \
    extern dev_t *g_lambda_console;                                             \
    mc68681_start_counter(g_lambda_console, (MC68681_CLK_HZ / 16) / TICK_RATE); \
}


/*
    plat_stop_quantum() - take any action necessary to finish the previous process time-slice.

    Note: this function will be called in interrupt context.

    This function is inlined, as it is used during context-switching and is therefore
    performance-critical.
*/
#define plat_stop_quantum()                                                     \
{                                                                               \
    extern dev_t *g_lambda_console;                                             \
    mc68681_stop_counter(g_lambda_console);                                     \
}


#define PLAT_DO_RESET                                                           \
    asm volatile                                                                \
    (                                                                           \
        "reset                                              \n"                 \
        "lea.l      " STRINGIFY(LAMBDA_ROM_START) ", %%a0   \n"                 \
        "move.l     %%a0@, %%a7                             \n"                 \
        "addq.l     #4, %%a0\n                              \n"                 \
        "move.l     %%a0@, %%a0                             \n"                 \
        "jmp        %%a0@                                   \n"                 \
        :                                                                       \
        :                                                                       \
    )

#endif
