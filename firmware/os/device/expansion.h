#ifndef DEVICE_EXPANSION_H_INC
#define DEVICE_EXPANSION_H_INC
/*
    Expansion card slot management

    Part of the as-yet-unnamed MC68010 operating system


    (c) Stuart Wallace, July 2015
*/

#include "cpu/exceptions.h"
#include "device/duart.h"
#include "include/defs.h"
#include "include/types.h"
#include "klibc/stdio.h"


#define EXP_BASE_ADDR               ((void *) (0xa00000))   /* Base addr of first expansion slot  */
#define EXP_ADDR_LEN                (0x100000)              /* Memory range assigned to each slot */

#define EXP_BASE_IRQ                (V_level_4_autovector)  /* IRQ assigned to slot 0             */

#define EXP_NUM_SLOTS               (4)

#define EXP_BASE(slot)              ((void *) (EXP_BASE_ADDR + (slot * EXP_ADDR_LEN)))


/*
    Expansion slot "presence detect" bits
*/

#define EXP_PD_REG                  (DUART_IP)

#define EXP_FIRST_PD_BIT            (2)     /* EXP0PD is connected to DUART pin IP2 */
#define EXP_PD_BIT(x)               (EXP_FIRST_PD_BIT + (x))
#define EXP_PD_MASK(x)              (1 << EXP_PD_BIT(x))
#define EXP_PD_ALL_MASK             (EXP_PD_MASK(3) | EXP_PD_MASK(2) | \
                                     EXP_PD_MASK(1) | EXP_PD_MASK(0))
#define EXP_PRESENT(slot)           (!((EXP_PD_REG) & EXP_PD_MASK(slot)))

/*
    Expansion slot "identify" (EID) line
*/

#define EXP_ID                      (2)
#define EXP_ID_MASK                 BIT(EXP_ID)
#define EXP_ID_ASSERT()             (DUART_SOPR = (EXP_ID_MASK))
#define EXP_ID_NEGATE()             (DUART_ROPR = (EXP_ID_MASK))


typedef struct
{
    void *base;
    u32 len;
    u32 irql;
} expansion_root_t;

void expansion_init();

#endif
