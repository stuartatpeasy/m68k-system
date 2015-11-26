#ifndef DRIVER_DS17485_H_INC
#define DRIVER_DS17485_H_INC
/*
    Dallas DS17485 driver function and macro declarations

    Part of the as-yet-unnamed MC68010 operating system


    (c) Stuart Wallace, June 2015.
*/

#include <kernel/device/device.h>
#include <kernel/include/defs.h>
#include <kernel/include/types.h>


/*
    OFFSET and SHIFT are used to deal with the situations where the DS17485 (an 8-bit device) is
    connected to one byte's worth of a wider data bus.  On the MC68000/010, with a 16-bit data bus,
    the device's registers will appear at every other address (requiring DS17485_SHIFT = 1).  If
    the device is connected to the upper half of the bus, its registers will appear at even
    addresses (DS17485_OFFSET = 0); if connected to the lower half of the bus, its registers will
    appear at odd addresses (DS17485_OFFSET = 1).
*/
#define DS17485_OFFSET                      (1)
#define DS17485_SHIFT                       (1)

/* Data register is at DS17485 "address" 0; address register is at "address" 1. */
#define DS17485_D_REG                       ((0 << DS17485_SHIFT) + DS17485_OFFSET)
#define DS17485_A_REG                       ((1 << DS17485_SHIFT) + DS17485_OFFSET)

/* Yields pointer to DS17485 address register   */
#define DS17485_ADDR(base)      *((vu8 *) (((u32) (base)) + DS17485_A_REG))

/* Yields pointer to DS17485 data register      */
#define DS17485_DATA(base)      *((vu8 *) (((u32) (base)) + DS17485_D_REG))

/* Read from register r */
#define DS17485_REG_READ(base, r)           \
	(DS17485_ADDR(base) = (r)), DS17485_DATA(base)

/* Write data to register r */
#define DS17485_REG_WRITE(base, r, data)    \
	(DS17485_ADDR(base) = (r)), (DS17485_DATA(base) = (data))

/* Set bits b in register r */
#define DS17485_REG_SET_BITS(base, r, b)    \
	(DS17485_ADDR(base) = (r)), (DS17485_DATA(base) |= (b))

/* Clear bits b in register r */
#define DS17485_REG_CLEAR_BITS(base, r, b)  \
	(DS17485_ADDR(base) = (r)), (DS17485_DATA(base) &= ~(b))

/* Switch to the DS17485 extended register set */
#define DS17485_SELECT_EXT_REG(base)        \
    DS17485_REG_SET_BITS(base, DS17485_REG_A, DS17485_DV0)

/* Switch to the DS17485 standard register set */
#define DS17485_SELECT_STD_REG(base)        \
    DS17485_REG_CLEAR_BITS(base, DS17485_REG_A, DS17485_DV0)

/*
    DS17485 register numbers
*/
#define DS17485_SECONDS                 (0x00)
#define DS17485_SECONDS_ALARM           (0x01)
#define DS17485_MINUTES                 (0x02)
#define DS17485_MINUTES_ALARM           (0x03)
#define DS17485_HOURS                   (0x04)
#define DS17485_HOURS_ALARM             (0x05)
#define DS17485_DAY_OF_WEEK             (0x06)
#define DS17485_DAY                     (0x07)
#define DS17485_MONTH                   (0x08)
#define DS17485_YEAR                    (0x09)

#define DS17485_REG_A                   (0x0A)
#define DS17485_REG_B                   (0x0B)
#define DS17485_REG_C                   (0x0C)
#define DS17485_REG_D                   (0x0D)

#define DS17485_MODEL_NUMBER            (0x40)

#define DS17485_SERIAL_NUM_1            (0x41)
#define DS17485_SERIAL_NUM_2            (0x42)
#define DS17485_SERIAL_NUM_3            (0x43)
#define DS17485_SERIAL_NUM_4            (0x44)
#define DS17485_SERIAL_NUM_5            (0x45)
#define DS17485_SERIAL_NUM_6            (0x46)

#define DS17485_REG_4A                  (0x4A)
#define DS17485_REG_4B                  (0x4B)

#define DS17485_CENTURY                 (0x48)
#define DS17485_DATE_ALARM              (0x49)

#define DS17485_EXTRAM_LSB              (0x50)
#define DS17485_EXTRAM_MSB              (0x51)
#define DS17485_EXTRAM_DATA             (0x53)


/* Register A bits */
#define DS17485_UIP             BIT(7)  /* Update in progress                           */
#define DS17485_DV2             BIT(6)  /* 0 = enable countdown chain; 1 = reset if DV1 */
#define DS17485_DV1             BIT(5)  /* Oscillator enable                            */
#define DS17485_DV0             BIT(4)  /* Register bank select: 0 = original, 1 = ext. */
#define DS17485_RS3             BIT(3)  /* } Square-wave output rate select             */
#define DS17485_RS2             BIT(2)  /* } see data sheet for frequencies             */
#define DS17485_RS1             BIT(1)  /* }                                            */
#define DS17485_RS0             BIT(0)  /* }                                            */

/* Register B bits */
#define DS17485_SET             BIT(7)  /* 1 = Inhibit update transfers during init.    */
#define DS17485_PIE             BIT(6)  /* Periodic interrupt enable                    */
#define DS17485_AIE             BIT(5)  /* Alarm interrupt enable                       */
#define DS17485_UIE             BIT(4)  /* Update-ended interrupt enable                */
#define DS17485_SQWE            BIT(3)  /* Square-wave output enable                    */
#define DS17485_DM              BIT(2)  /* Data mode: 1 = binary, 0 = BCD               */
#define DS17485_2412            BIT(1)  /* 1 = 24-hour mode, 0 = 12-hour mode           */
#define DS17485_DSE             BIT(0)  /* Daylight savings enable                      */

/* Register C bits */
#define DS17485_IRQF            BIT(7)  /* Interrupt request flag                       */
#define DS17485_PF              BIT(6)  /* Periodic interrupt flag                      */
#define DS17485_AF              BIT(5)  /* Alarm interrupt flag                         */
#define DS17485_UF              BIT(4)  /* Update ended interrupt flag                  */

/* Register D bits */
#define DS17485_VRT             BIT(7)  /* Valid RAM and time (Vbat and Vbaux good)     */

/* Register 4A bits */
#define DS17485_VRT2            BIT(7)  /* Valid RAM and time (Vbaux good only)         */
#define DS17485_INCR            BIT(6)  /* Increment in progress                        */
#define DS17485_BME             BIT(5)  /* Burst mode enable                            */
#define DS17485_PAB             BIT(3)  /* Power-active-bar control                     */
#define DS17485_RF              BIT(2)  /* Ram clear flag                               */
#define DS17485_WF              BIT(1)  /* Wake-up alarm flag                           */
#define DS17485_KF              BIT(0)  /* Kickstart flag                               */

/* Register 4B bits */
#define DS17485_ABE             BIT(7)  /* Auxiliary battery enable                     */
#define DS17485_E32K            BIT(6)  /* Enable 32.768kHz output on SQW pin           */
#define DS17485_CS              BIT(5)  /* Crystal select: 0 = 6pF, 1 = 12pF load cap   */
#define DS17485_RCE             BIT(4)  /* RAM clear enable                             */
#define DS17485_PRS             BIT(3)  /* PAB reset select                             */
#define DS17485_RIE             BIT(2)  /* RAM clear interrupt enable                   */
#define DS17485_WIE             BIT(1)  /* Wake-up alarm interrupt enable               */
#define DS17485_KSE             BIT(0)  /* Kickstart interrupt enable                   */


#define DS17485_USER_RAM_LEN    (114)   /* Length of user RAM area, in bytes            */
#define DS17485_USER_RAM_START  (14)    /* Address of first byte of user RAM            */

#define DS17485_EXT_RAM_LEN     (4096)  /* Length of extended RAM area, in bytes        */

/*
    Function declarations
*/
s32 ds17485_init(dev_t * const dev);
s32 ds17485_rtc_init(dev_t * const dev);
s32 ds17485_user_ram_init(dev_t * const dev);
s32 ds17485_ext_ram_init(dev_t * const dev);

s32 ds17485_rtc_read(dev_t * const dev, ku32 offset, u32 *len, void *buffer);
s32 ds17485_rtc_write(dev_t * const dev, ku32 offset, u32 *len, const void *buffer);
void ds17485_force_valid_time(const dev_t * const dev);

s32 ds17485_user_ram_read(dev_t * const dev, u32 addr, u32 *len, void * buffer);
s32 ds17485_user_ram_write(dev_t * const dev, u32 addr, u32 *len, const void * buffer);

s32 ds17485_ext_ram_read(dev_t * const dev, u32 addr, u32 *len, void * buffer);
s32 ds17485_ext_ram_write(dev_t * const dev, u32 addr, u32 *len, const void * buffer);

u8 ds17485_get_model_number(const dev_t * const dev);
void ds17485_get_serial_number(const dev_t * const dev, u8 sn[6]);


#endif // DS17485_H_INCLUDED
