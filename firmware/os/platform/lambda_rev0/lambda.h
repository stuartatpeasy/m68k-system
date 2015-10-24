#ifndef PLATFORM_LAMBDA_REV0_LAMBDA_H_INC
#define PLATFORM_LAMBDA_REV0_LAMBDA_H_INC
/*
    ayumos port for the "lambda" rev0 (MC68010) motherboard

    Stuart Wallace, September 2015
*/

#include <device/device.h>
#include <device/mc68681.h>

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

#endif
