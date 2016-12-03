#ifndef BOARD_LAMBDA_DEVICE_H_INC
#define BOARD_LAMBDA_DEVICE_H_INC
/*
    Hardware device enumeration for the "lambda" (MC68010) motherboard

    Stuart Wallace, September 2015
*/

#include <cpu/mc68000/exceptions.h>

#include <kernel/device/device.h>
#include <kernel/include/defs.h>
#include <kernel/include/types.h>


/*
    Hardware base addresses and IRQ levels
*/
#define LAMBDA_MC68681_BASE         ((void *) 0xe00000)
#define LAMBDA_MC68681_IRQL         (27)

#define LAMBDA_DS17485_BASE         ((void *) 0xe10000)
#if (PLATFORM_REV == 0)
    #define LAMBDA_DS17485_IRQL     (IRQL_NONE)
#else
    #define LAMBDA_DS17485_IRQL     (28)
#endif

#define LAMBDA_ATA_BASE             ((void *) 0xe20000)
#define LAMBDA_ATA_IRQL             (26)


/*
    Expansion slot base addresses and IRQ levels
*/
#define LAMBDA_EXP_BASE_ADDR        ((void *) (0xa00000))   /* Base addr of first expansion slot  */
#define LAMBDA_EXP_ADDR_LEN         (0x100000)              /* Memory range assigned to each slot */

#if (PLATFORM_REV == 0)
    /* Revision 0 uses one IRQ per slot, starting at IRQ4 (irql 28) */
    #define LAMBDA_EXP_BASE_IRQ     (28)                    /* IRQ assigned to slot 0             */
    #define LAMBDA_EXP_IRQ(slot) \
        (LAMBDA_EXP_BASE_IRQ + (slot))
#else
    /* Revision 1 onwards uses IRQ5 (irql 29) for slots 0 & 1, and IRQ6 (irql 30) for slots 2 & 3 */
    #define LAMBDA_EXP_BASE_IRQ     (29)
    #define LAMBDA_EXP_IRQ(slot) \
        (LAMBDA_EXP_BASE_IRQ + (slot >> 1))
#endif

#define LAMBDA_EXP_NUM_SLOTS        (4)

#define LAMBDA_EXP_BASE(slot)       ((void *) (LAMBDA_EXP_BASE_ADDR + (slot * LAMBDA_EXP_ADDR_LEN)))


/*
    Expansion slot "presence detect" bits
*/

#define LAMBDA_EXP_FIRST_PD_BIT     (2)     /* EXP0PD is connected to DUART pin IP2 */
#define LAMBDA_EXP_PD_BIT(x)        (LAMBDA_EXP_FIRST_PD_BIT + (x))
#define LAMBDA_EXP_PD_MASK(x)       BIT(LAMBDA_EXP_PD_BIT(x))
#define EXP_PD_ALL_MASK             (LAMBDA_EXP_PD_MASK(3) | LAMBDA_EXP_PD_MASK(2) | \
                                     LAMBDA_EXP_PD_MASK(1) | LAMBDA_EXP_PD_MASK(0))

/*
    Expansion slot "identify" (EID) line
*/

#define LAMBDA_EXP_ID               (2)


/*
    MC68681 output port bit connected to the beeper
*/
#define LAMBDA_DUART_BEEPER_OUTPUT  (5)


dev_t *g_lambda_duart;
dev_t *g_lambda_console;

void expansion_init();

#endif
