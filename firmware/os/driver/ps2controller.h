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
#include <kernel/util/buffer.h>


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
    ((vu8 *) (((u32) (base)) + PS2CTRLR_BASE + ((r) << PS2CTRLR_SHIFT) + PS2CTRLR_OFFSET))

/* Accessor for register "r" in a peripheral based at address "base" */
#define PS2CTRLR_REG(base, r)    (*PS2CTRLR_REG_ADDR((base), (r)))


/* Flags associated with packets of data received from a port */
#define PS2_PKT_KB_RELEASE      BIT(7)  /* Key-release prefix received                          */
#define PS2_PKT_KB_EXT1         BIT(6)  /* First extended-key prefix flag (0xe0) received       */
#define PS2_PKT_KB_EXT2         BIT(5)  /* Second extended-key prefix flag (0xe1) received      */

/* "Special" keyboard scan code component bytes */
#define PS2_SC_KB_RELEASE       (0xf0)  /* Key-release event                                    */
#define PS2_SC_KB_EXT1          (0xe0)  /* First extended key set                               */
#define PS2_SC_KB_EXT2          (0xe1)  /* Second extended key set                              */


/* State of a single PS/2 port */
typedef struct
{
    struct
    {
        vu8 *   data;
        vu8 *   status;
        vu8 *   int_cfg;
    } regs;

    struct
    {
        struct
        {
            u32     data;
            u8      flags;
        } kb;
    } packet;

    u8                      err;
    CIRCBUF(u8)             tx_buf;
} ps2controller_port_state_t;


/* Overall controller state */
typedef struct
{
    dev_t   *port_a;
    dev_t   *port_b;
} ps2controller_state_t;


/* PS/2 port state machine states */
enum PS2Port_State
{
    PS2_STATE_INIT,
    PS2_STATE_RESET_SENT,
    PS2_STATE_RESET_ACK_RECEIVED,

};


/*
    PS/2 commands.  Note that many of these apply only to a keyboard, or only to a mouse
*/
/* PS/2 commands which apply to keyboard and mouse */
#define PS2_CMD_RESET                   (0xff)
#define PS2_CMD_RESEND                  (0xfe)
#define PS2_CMD_SET_DEFAULTS            (0xf6)
#define PS2_CMD_DISABLE                 (0xf5)
#define PS2_CMD_ENABLE                  (0xf4)
#define PS2_CMD_READ_ID                 (0xf2)

/* PS/2 keyboard-specific commands */
#define PS2_CMD_SET_TYPEMATIC_RATE      (0xf3)
#define PS2_CMD_SET_KEY_TYPE_MAKE       (0xfd)
#define PS2_CMD_SET_KEY_TYPE_MAKE_BREAK (0xfc)
#define PS2_CMD_SET_KEY_TYPE_TYPEMATIC  (0xfb)
#define PS2_CMD_SET_ALL_KEYS_NORMAL     (0xfa)
#define PS2_CMD_SET_ALL_KEYS_MAKE       (0xf9)
#define PS2_CMD_SET_ALL_KEYS_MAKE_BREAK (0xf8)
#define PS2_CMD_SET_ALL_KEYS_TYPEMATIC  (0xf7)
#define PS2_CMD_SET_SCAN_CODE_SET       (0xf0)
#define PS2_CMD_ECHO                    (0xee)
#define PS2_CMD_SET_LEDS                (0xed)

/* PS/2 mouse-specific commands */
#define PS2_CMD_SET_SAMPLE_RATE         (0xf3)
#define PS2_CMD_SET_REMOTE_MODE         (0xf0)
#define PS2_CMD_SET_WRAP_MODE           (0xee)
#define PS2_CMD_RESET_WRAP_MODE         (0xec)
#define PS2_CMD_READ_DATA               (0xeb)
#define PS2_CMD_SET_STREAM_MODE         (0xea)
#define PS2_CMD_STATUS_REQUEST          (0xe9)


/* Scan code sent to prefix a "key release" code */
#define PS2_SC_RELEASE                  (0xf0)


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


/* Port-specific register base address offset*/
#define PS2_PORT_A_REG_OFFSET   (0)
#define PS2_PORT_B_REG_OFFSET   (4)


/* Offsets of port-specific registers from port base address */
enum PS2Controller_PortRegOffset
{
    PS2_DATA    = 0x00,
    PS2_STATUS  = 0x01,
    PS2_INT_CFG = 0x02
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

/* Mask representing all possible error constants */
#define PS2_ERR_MASK    (PS2_FLAG_PAR_ERR | PS2_FLAG_OVF | PS2_FLAG_RX_TIMEOUT | \
                         PS2_FLAG_TX_TIMEOUT | PS2_FLAG_START_TIMEOUT | PS2_FLAG_CMD_TIMEOUT)

/* Bits in PS2_CFG */
#define PS2_CFG_IE              BIT(7)     /* Global interrupt enable                           */
#define PS2_CFG_PWR_A           BIT(4)     /* Enable/disable power to port A                    */
#define PS2_CFG_PWR_B           BIT(3)     /* Enable/disable power to port B                    */


s32 ps2controller_init(dev_t *dev);

#endif
