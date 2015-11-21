#ifndef BOARD_LAMBDA_DEVICE_H_INC
#define BOARD_LAMBDA_DEVICE_H_INC
/*
    Hardware device enumeration for the "lambda" (MC68010) motherboard

    Stuart Wallace, September 2015
*/

#include <cpu/mc68000/exceptions.h>

#include <include/defs.h>
#include <include/types.h>
#include <kernel/device/device.h>
#include <stdio.h>


/*
    Hardware base addresses and IRQ levels
*/
#define LAMBDA_MC68681_BASE         ((void *) 0xe00000)
#define LAMBDA_MC68681_IRQL         (27)

#define LAMBDA_DS17485_BASE         ((void *) 0xe10000)
#define LAMBDA_DS17485_IRQL         (IRQL_NONE)

#define LAMBDA_ATA_BASE             ((void *) 0xe20000)
#define LAMBDA_ATA_IRQL             (26)


/*
    Expansion slot base addresses and IRQ levels
*/
#define EXP_BASE_ADDR               ((void *) (0xa00000))   /* Base addr of first expansion slot  */
#define EXP_ADDR_LEN                (0x100000)              /* Memory range assigned to each slot */

#define EXP_BASE_IRQ                (V_level_4_autovector)  /* IRQ assigned to slot 0             */

#define EXP_NUM_SLOTS               (4)

#define EXP_BASE(slot)              ((void *) (EXP_BASE_ADDR + (slot * EXP_ADDR_LEN)))


/*
    Expansion slot "presence detect" bits
*/

#define EXP_FIRST_PD_BIT            (2)     /* EXP0PD is connected to DUART pin IP2 */
#define EXP_PD_BIT(x)               (EXP_FIRST_PD_BIT + (x))
#define EXP_PD_MASK(x)              (1 << EXP_PD_BIT(x))
#define EXP_PD_ALL_MASK             (EXP_PD_MASK(3) | EXP_PD_MASK(2) | \
                                     EXP_PD_MASK(1) | EXP_PD_MASK(0))

/*
    Expansion slot "identify" (EID) line
*/

#define EXP_ID                      (2)


dev_t *g_lambda_duart;
dev_t *g_lambda_console;

void expansion_init();

#endif
