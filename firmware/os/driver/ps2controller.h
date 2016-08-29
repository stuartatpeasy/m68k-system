#ifndef DRIVER_PS2CONTROLLER_H_INC
#define DRIVER_PS2CONTROLLER_H_INC
/*
    ATmega8-driven dual PS/2 port controller driver

    Part of the as-yet-unnamed MC68010 operating system


    (c) Stuart Wallace <stuartw@atom.net>, December 2015.

    This device has driver ID 0x82.
*/

#include <kernel/device/device.h>
#include <kernel/include/defs.h>
#include <kernel/include/error.h>
#include <kernel/include/types.h>


/*
    Register address-calculation macros.  In order to accommodate various different ways of
    connecting the controller to the host bus, the address of register r, relative to the base
    address of the PS/2 controller peripheral in memory, is calculated as:

        PS2CTRLR_BASE + (r << PS2CTRLR_SHIFT) + PS2CTRLR_OFFSET
*/
#define PS2CTRLR_BASE           (0)     /* Base address of PS/2 controller registers            */
#ifndef PS2CTRLR_SHIFT
#define PS2CTRLR_SHIFT          (1)     /* log2(gap_between_adjacent_registers)                 */
#endif
#ifndef PSCTRLR_OFFSET
#define PS2CTRLR_OFFSET         (0)     /* Offset of each register from its natural address     */
#endif

/* Given a peripheral based at address "base", obtain the address of register "r" */
#define PS2CTRLR_REG_ADDR(base, r)   \
    (((u32) (base)) + PS2CTRLR_BASE + ((r) << PS2CTRLR_SHIFT) + PS2CTRLR_OFFSET)

/* Accessor for register "r" in a peripheral based at address "base" */
#define PS2CTRLR_REG(base, r)    *((vu8 *) PS2CTRLR_REG_ADDR((base), (r)))

/* PS/2 controller registers */
enum PS2Controller_Reg
{
    PS2_DATA_A      = 0x00,
    PS2_STATUS_A    = 0x01,
    PS2_INT_CFG_A   = 0x02,
    PS2_CFG         = 0x03,
    PS2_DATA_B      = 0x04,
    PS2_STATUS_B    = 0x05,
    PS2_INT_CFG_B   = 0x06,
    PS2_UNUSED      = 0x07
};

/*
    Individual register bits
*/

/* Flags - used in PS2_STATUS and PS2_INT_CFG */
#define PS2_FLAG_RX             BIT(7)     /* Byte received                                     */
#define PS2_FLAG_TX             BIT(6)     /* Transmit finished                                 */
#define PS2_FLAG_PAR_ERR        BIT(5)     /* Receiver parity error                             */
#define PS2_FLAG_OVF            BIT(4)     /* Keyboard FIFO overflow                            */
#define PS2_FLAG_RX_TIMEOUT     BIT(3)     /* Timeout during receive                            */
#define PS2_FLAG_TX_TIMEOUT     BIT(2)     /* Timeout during transmit                           */
#define PS2_FLAG_START_TIMEOUT  BIT(1)     /* Timeout while waiting for TX clock                */
#define PS2_FLAG_CMD_TIMEOUT    BIT(0)     /* Timeout waiting for command response              */

/* Bits in PS2_CFG */
#define PS2_CFG_IE              BIT(7)     /* Global interrupt enable                           */
#define PS2_CFG_PWR_A           BIT(4)     /* Enable/disable power to channel A                 */
#define PS2_CFG_PWR_B           BIT(3)     /* Enable/disable power to channel B                  */


s32 ps2controller_init(dev_t *dev);

#endif
