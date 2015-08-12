#ifndef DEVICE_DS17485_H_INC
#define DEVICE_DS17485_H_INC
/*
    Dallas DS17485 driver function and macro declarations

    Part of the as-yet-unnamed MC68010 operating system


    (c) Stuart Wallace, June 2015.
*/

#include "include/types.h"

/*
    Base address of DS17485 IC
*/
#define DS17485_BASE        ((u8 *) 0x00e10000)

/* Yields pointer to DS17485 address register   */
#define DS17485_ADDR        *((vu8 *) (DS17485_BASE + 3))

/* Yields pointer to DS17485 data register      */
#define DS17485_DATA        *((vu8 *) (DS17485_BASE + 1))

/* Read from register r */
#define DS17485_REG_READ(r)             \
    ({                                  \
        DS17485_ADDR = r;               \
        DS17485_DATA;                   \
    })

/* Write data to register r */
#define DS17485_REG_WRITE(r, data)      \
    ({                                  \
        DS17485_ADDR = r;               \
        DS17485_DATA = data;            \
    })

/* Set bits b in register r */
#define DS17485_REG_SET_BITS(r, b)      \
    ({                                  \
        DS17485_ADDR = r;               \
        DS17485_DATA |= b;              \
    })

/* Clear bits b in register r */
#define DS17485_REG_CLEAR_BITS(r, b)    \
    ({                                  \
        DS17485_ADDR = r;               \
        DS17485_DATA &= ~b;             \
    })

/* Switch to the DS17485 extended register set */
#define DS17485_SELECT_EXT_REG()        \
    DS17485_REG_SET_BITS(DS17485_REG_A, DS17485_DV0)

/* Switch to the DS17485 standard register set */
#define DS17485_SELECT_STD_REG()        \
    DS17485_REG_CLEAR_BITS(DS17485_REG_A, DS17485_DV0)


/*
    Function declarations
*/
void ds17485_init();
void ds17485_get_time(rtc_time_t * const tm);
void ds17485_set_time(const rtc_time_t * const tm);
void ds17485_force_valid_time();
void ds17485_user_ram_read(u32 addr, u32 len, void * buffer);
void ds17485_user_ram_write(u32 addr, u32 len, const void * buffer);
void ds17485_ext_ram_read(u32 addr, u32 len, u8* buffer);
void ds17485_ext_ram_write(u32 addr, u32 len, const u8* buffer);
u8 ds17485_get_model_number();
void ds17485_get_serial_number(u8 sn[6]);

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
#define DS17485_UIP                     (1 << 7)
#define DS17485_DV2                     (1 << 6)
#define DS17485_DV1                     (1 << 5)
#define DS17485_DV0                     (1 << 4)
#define DS17485_RS3                     (1 << 3)
#define DS17485_RS2                     (1 << 2)
#define DS17485_RS1                     (1 << 1)
#define DS17485_RS0                     (1 << 0)

/* Register B bits */
#define DS17485_SET                     (1 << 7)
#define DS17485_PIE                     (1 << 6)
#define DS17485_AIE                     (1 << 5)
#define DS17485_UIE                     (1 << 4)
#define DS17485_SQWE                    (1 << 3)
#define DS17485_DM                      (1 << 2)
#define DS17485_2412                    (1 << 1)
#define DS17485_DSE                     (1 << 0)

/* Register C bits */
#define DS17485_IRQF                    (1 << 7)
#define DS17485_PF                      (1 << 6)
#define DS17485_AF                      (1 << 5)
#define DS17485_UF                      (1 << 4)

/* Register D bits */
#define DS17485_VRT                     (1 << 7)

/* Register 4A bits */
#define DS17485_VRT2                    (1 << 7)
#define DS17485_INCR                    (1 << 6)
#define DS17485_BME                     (1 << 5)
#define DS17485_PAB                     (1 << 3)
#define DS17485_RF                      (1 << 2)
#define DS17485_WF                      (1 << 1)
#define DS17485_KF                      (1 << 0)

/* Register 4B bits */
#define DS17485_ABE                     (1 << 7)
#define DS17485_E32K                    (1 << 6)
#define DS17485_CS                      (1 << 5)
#define DS17485_RCE                     (1 << 4)
#define DS17485_PRS                     (1 << 3)
#define DS17485_RIE                     (1 << 2)
#define DS17485_WIE                     (1 << 1)
#define DS17485_KSE                     (1 << 0)


#endif // DS17485_H_INCLUDED
