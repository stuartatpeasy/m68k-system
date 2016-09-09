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
        vu8 *       data;
        vu8 *       status;
        vu8 *       int_cfg;
    } regs;

    union
    {
        struct
        {
            u32     data;
            u8      flags;
        } kb;

        struct
        {
            s16     dx;
            s16     dy;
            s16     dz;
            u8      buttons;
        } mouse;
    } packet;

    u8              err;
    CIRCBUF(u8)     tx_buf;
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


/*
    Constants representing PS/2 set 2 single-byte keyboard scan codes.
*/
enum PS2ScanCodes_Set2
{
    PS2_SC2_KEY_A           = 0x1c,
    PS2_SC2_KEY_B           = 0x32,
    PS2_SC2_KEY_C           = 0x21,
    PS2_SC2_KEY_D           = 0x23,
    PS2_SC2_KEY_E           = 0x24,
    PS2_SC2_KEY_F           = 0x2b,
    PS2_SC2_KEY_G           = 0x34,
    PS2_SC2_KEY_H           = 0x33,
    PS2_SC2_KEY_I           = 0x43,
    PS2_SC2_KEY_J           = 0x3b,
    PS2_SC2_KEY_K           = 0x42,
    PS2_SC2_KEY_L           = 0x4b,
    PS2_SC2_KEY_M           = 0x3a,
    PS2_SC2_KEY_N           = 0x31,
    PS2_SC2_KEY_O           = 0x44,
    PS2_SC2_KEY_P           = 0x4d,
    PS2_SC2_KEY_Q           = 0x15,
    PS2_SC2_KEY_R           = 0x2d,
    PS2_SC2_KEY_S           = 0x1b,
    PS2_SC2_KEY_T           = 0x2c,
    PS2_SC2_KEY_U           = 0x3c,
    PS2_SC2_KEY_V           = 0x2a,
    PS2_SC2_KEY_W           = 0x1d,
    PS2_SC2_KEY_X           = 0x22,
    PS2_SC2_KEY_Y           = 0x35,
    PS2_SC2_KEY_Z           = 0x1a,

    PS2_SC2_KEY_0           = 0x45,
    PS2_SC2_KEY_1           = 0x16,
    PS2_SC2_KEY_2           = 0x1e,
    PS2_SC2_KEY_3           = 0x26,
    PS2_SC2_KEY_4           = 0x25,
    PS2_SC2_KEY_5           = 0x2e,
    PS2_SC2_KEY_6           = 0x36,
    PS2_SC2_KEY_7           = 0x3d,
    PS2_SC2_KEY_8           = 0x3e,
    PS2_SC2_KEY_9           = 0x46,

    PS2_SC2_KEY_F1          = 0x05,
    PS2_SC2_KEY_F2          = 0x06,
    PS2_SC2_KEY_F3          = 0x04,
    PS2_SC2_KEY_F4          = 0x0c,
    PS2_SC2_KEY_F5          = 0x03,
    PS2_SC2_KEY_F6          = 0x0b,
    PS2_SC2_KEY_F7          = 0x83,
    PS2_SC2_KEY_F8          = 0x0a,
    PS2_SC2_KEY_F9          = 0x01,
    PS2_SC2_KEY_F10         = 0x09,
    PS2_SC2_KEY_F11         = 0x78,
    PS2_SC2_KEY_F12         = 0x07,

    PS2_SC2_KEY_KP_AST      = 0x7c,
    PS2_SC2_KEY_KP_MINUS    = 0x7b,
    PS2_SC2_KEY_KP_PLUS     = 0x79,
    PS2_SC2_KEY_KP_DOT      = 0x71,
    PS2_SC2_KEY_KP_0        = 0x70,
    PS2_SC2_KEY_KP_1        = 0x69,
    PS2_SC2_KEY_KP_2        = 0x72,
    PS2_SC2_KEY_KP_3        = 0x7a,
    PS2_SC2_KEY_KP_4        = 0x6b,
    PS2_SC2_KEY_KP_5        = 0x73,
    PS2_SC2_KEY_KP_6        = 0x74,
    PS2_SC2_KEY_KP_7        = 0x6c,
    PS2_SC2_KEY_KP_8        = 0x75,
    PS2_SC2_KEY_KP_9        = 0x7d,

    PS2_SC2_KEY_CAPS        = 0x58,
    PS2_SC2_KEY_NUM         = 0x77,
    PS2_SC2_KEY_SCROLL      = 0x7e,

    PS2_SC2_KEY_APOST       = 0x52,
    PS2_SC2_KEY_BKTICK      = 0x0e,
    PS2_SC2_KEY_BKSLASH     = 0x5d,
    PS2_SC2_KEY_BKSP        = 0x66,
    PS2_SC2_KEY_COMMA       = 0x41,
    PS2_SC2_KEY_DOT         = 0x49,
    PS2_SC2_KEY_ENTER       = 0x5a,
    PS2_SC2_KEY_EQUALS      = 0x55,
    PS2_SC2_KEY_ESC         = 0x76,
    PS2_SC2_KEY_FWDSLASH    = 0x4a,
    PS2_SC2_KEY_HYPHEN      = 0x4e,
    PS2_SC2_KEY_SCOLON      = 0x4c,
    PS2_SC2_KEY_SPACE       = 0x29,
    PS2_SC2_KEY_TAB         = 0x0d,
    PS2_SC2_KEY_L_BKT       = 0x54,
    PS2_SC2_KEY_R_BKT       = 0x5b,

    PS2_SC2_KEY_L_SHIFT     = 0x12,
    PS2_SC2_KEY_L_CTRL      = 0x14,
    PS2_SC2_KEY_L_ALT       = 0x11,
    PS2_SC2_KEY_R_SHFT      = 0x59
};


/*
    Constants representing PS/2 set 2 scan codes prefixed by a 0xe0 byte (we call this the "first
    extended" set)
*/
enum PS2ScanCodes_Set2_Ext1
{
    PS2_SC2_KEY_L_GUI       = 0x1f,
    PS2_SC2_KEY_R_CTRL      = 0x14,
    PS2_SC2_KEY_R_GUI       = 0x27,
    PS2_SC2_KEY_R_ALT       = 0x11,
    PS2_SC2_KEY_APPS        = 0x2f,
    PS2_SC2_KEY_INSERT      = 0x70,
    PS2_SC2_KEY_HOME        = 0x6c,
    PS2_SC2_KEY_PG_UP       = 0x7d,
    PS2_SC2_KEY_DELETE      = 0x71,
    PS2_SC2_KEY_END         = 0x69,
    PS2_SC2_KEY_PG_DN       = 0x7a,
    PS2_SC2_KEY_U_ARROW     = 0x75,
    PS2_SC2_KEY_L_ARROW     = 0x6b,
    PS2_SC2_KEY_D_ARROW     = 0x72,
    PS2_SC2_KEY_R_ARROW     = 0x74,
    PS2_SC2_KEY_KP_FWDSLASH = 0x4a,
    PS2_SC2_KEY_KP_ENTER    = 0x5a,
    PS2_SC2_KEY_POWER       = 0x37,
    PS2_SC2_KEY_SLEEP       = 0x3f,
    PS2_SC2_KEY_WAKE        = 0x5e,
};

/* Other PS/2 set 2 scan codes */
#define PS2_SC2_PRTSC_PRESS     (0x127c)        /* PrtSc pressed: actual code e0 12 e0 7c */
#define PS2_SC2_PRTSC_RELEASE   (0x7c12)        /* PrtSc released: actual code e0 f0 7c e0 f0 12 */
#define PS2_SC2_PAUSE           (0x14771477)    /* Pause: actual code e1 14 77 e1 f0 14 f0 77 */


s32 ps2controller_init(dev_t *dev);

#endif
