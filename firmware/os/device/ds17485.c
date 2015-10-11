/*
    Dallas DS17485 driver function definitions

    Part of the as-yet-unnamed MC68010 operating system


    (c) Stuart Wallace, June 2015.
*/

#include "ds17485.h"
#include "kutil/kutil.h"


/*
    ds17485_init() - initialise a DS17485 RTC.
*/
s32 ds17485_init(const dev_t * const dev)
{
    /*
        Write register A:
            UIP   = 0       (ignored; read only)
            DV2   = 0       Enable countdown chain
            DV1   = 1       Oscillator on, VCC power-up state
            DV0   = 1       Select extended register bank
            RS3   = 0       }
            RS2   = 0       } disable periodic interrupts;
            RS1   = 0       } disable square-wave output
            RS0   = 0       }
    */
    DS17485_REG_WRITE(dev->base_addr, DS17485_REG_A, DS17485_DV1 | DS17485_DV0);

    /* Write register 4A: enable extended RAM burst mode */
    DS17485_REG_WRITE(dev->base_addr, DS17485_REG_4A, DS17485_BME);

    /* Write register 4B: enable RAM clear input pin; select 12.5pF crystal load capacitance */
    DS17485_REG_WRITE(dev->base_addr, DS17485_REG_4B, DS17485_RCE | DS17485_CS);

    /* Write register A: select standard registers */
    DS17485_REG_WRITE(dev->base_addr, DS17485_REG_A, DS17485_DV1);

    /*
        Write register B.  We may end up changing the data format with this write, so it's
        necessary to write twice: once with the SET bit asserted, and one with it negated.

        First write:
            SET   = 0       Don't inhibit updates
            PIE   = 0       Disable periodic interrupts
            AIE   = 0       Disable alarm interrupt
            UIE   = 0       Disable update-end interrupt
            SQWE  = 0       Disable square-wave output
            DM    = 1       Set data format to binary (vs. BCD)
            24/12 = 1       Use 24h format
            DSE   = 0       Disable daylight saving time
    */
    DS17485_REG_WRITE(dev->base_addr, DS17485_REG_B, DS17485_DM | DS17485_2412 | DS17485_SET);

    /* Second write - same as first, but with SET bit negated. */
    DS17485_REG_WRITE(dev->base_addr, DS17485_REG_B, DS17485_DM | DS17485_2412);

    ds17485_force_valid_time(dev);

    return SUCCESS;
}


/*
    ds17485_get_time() - read the current time and populate a struct rtc_time_t.
*/
void ds17485_get_time(const dev_t * const dev, rtc_time_t * const tm)
{
    /* Set the "SET" bit in register B, to prevent updates while we read */
    DS17485_REG_SET_BITS(dev->base_addr, DS17485_REG_B, DS17485_SET);

    /* Need to switch to extended register set in order to read the century */
    DS17485_SELECT_EXT_REG(dev->base_addr);

    tm->year = 100 * DS17485_REG_READ(dev->base_addr, DS17485_CENTURY);

    /* Switch back to the standard register set to read the rest of the date/time */
    DS17485_SELECT_STD_REG(dev->base_addr);

    tm->hour = DS17485_REG_READ(dev->base_addr, DS17485_HOURS);
    tm->minute = DS17485_REG_READ(dev->base_addr, DS17485_MINUTES);
    tm->second = DS17485_REG_READ(dev->base_addr, DS17485_SECONDS);

    tm->day_of_week = DS17485_REG_READ(dev->base_addr, DS17485_DAY_OF_WEEK);
    tm->day = DS17485_REG_READ(dev->base_addr, DS17485_DAY);
    tm->month = DS17485_REG_READ(dev->base_addr, DS17485_MONTH);
    tm->year += DS17485_REG_READ(dev->base_addr, DS17485_YEAR);

    tm->dst = DS17485_REG_READ(dev->base_addr, DS17485_REG_B) & DS17485_DSE;

    /* Clear the "SET" bit in register B, as we have finished reading data */
    DS17485_REG_CLEAR_BITS(dev->base_addr, DS17485_REG_B, DS17485_SET);
}


/*
    ds17485_set_time() - set the time stored in a DS17485 to the time specified by tm.
*/
void ds17485_set_time(const dev_t * const dev, const rtc_time_t * const tm)
{
    /* Set the "SET" bit in register B, to prevent updates while we write */
    DS17485_REG_SET_BITS(dev->base_addr, DS17485_REG_B, DS17485_SET);

    /* Need to switch to extended register set in order to write the century */
    DS17485_SELECT_EXT_REG(dev->base_addr);

    DS17485_REG_WRITE(dev->base_addr, DS17485_CENTURY, tm->year / 100);

    /* Switch back to the standard register set to read the rest of the date/time */
    DS17485_SELECT_STD_REG(dev->base_addr);

    DS17485_REG_WRITE(dev->base_addr, DS17485_YEAR, tm->year % 100);

    DS17485_REG_WRITE(dev->base_addr, DS17485_MONTH, tm->month);
    DS17485_REG_WRITE(dev->base_addr, DS17485_DAY, tm->day);
    DS17485_REG_WRITE(dev->base_addr, DS17485_HOURS, tm->hour);
    DS17485_REG_WRITE(dev->base_addr, DS17485_MINUTES, tm->minute);
    DS17485_REG_WRITE(dev->base_addr, DS17485_SECONDS, tm->second);

    DS17485_REG_WRITE(dev->base_addr, DS17485_DAY_OF_WEEK, tm->day_of_week);

    DS17485_REG_SET_BITS(dev->base_addr, DS17485_REG_B, (tm->dst > 0));

    /* Clear the "SET" bit in register B, as we have finished writing data */
    DS17485_REG_CLEAR_BITS(dev->base_addr, DS17485_REG_B, DS17485_SET);
}


void ds17485_force_valid_time(const dev_t * const dev)
{
    rtc_time_t ts;

    ds17485_get_time(dev->base_addr, &ts);

    if(!VALID_RTC_DATE(&ts))
    {
        /* Date/time is invalid; set to 2000-01-01 00:00:00 (Saturday) */
        ts.year = 2000;
        ts.month = 1;
        ts.day = 1;
        ts.hour = 0;
        ts.minute = 0;
        ts.second = 0;
        ts.day_of_week = 7;
        ts.dst = 0;

        ds17485_set_time(dev->base_addr, &ts);
    }
}


void ds17485_user_ram_read(const dev_t * const dev, u32 addr, u32 len, void* buffer)
{
    DS17485_SELECT_STD_REG(dev->base_addr);
    for(addr += 14; len && (addr < 128); --len, ++addr)
        *((u8 *) buffer++) = DS17485_REG_READ(dev->base_addr, addr);
}


void ds17485_user_ram_write(const dev_t * const dev, u32 addr, u32 len, const void* buffer)
{
    DS17485_SELECT_STD_REG(dev->base_addr);
    for(addr += 14; len && (addr < 128); --len, ++addr)
        DS17485_REG_WRITE(dev->base_addr, addr, *((u8 *) buffer++));
}


void ds17485_ext_ram_read(const dev_t * const dev, u32 addr, u32 len, u8* buffer)
{
    /* Switch to the extended register set in order to read the extended RAM area */
    DS17485_SELECT_EXT_REG(dev->base_addr);

    DS17485_REG_WRITE(dev->base_addr, DS17485_EXTRAM_MSB, (addr & 0xf00) >> 8);
    DS17485_REG_WRITE(dev->base_addr, DS17485_EXTRAM_LSB, addr & 0xff);

    while(len-- & (addr++ < 0xfff))
        *(buffer++) = DS17485_REG_READ(dev->base_addr, DS17485_EXTRAM_DATA);
}


void ds17485_ext_ram_write(const dev_t * const dev, u32 addr, u32 len, const u8* buffer)
{
    /* Switch to the extended register set in order to read the extended RAM area */
    DS17485_SELECT_EXT_REG(dev->base_addr);

    DS17485_REG_WRITE(dev->base_addr, DS17485_EXTRAM_MSB, (addr & 0xf00) >> 8);
    DS17485_REG_WRITE(dev->base_addr, DS17485_EXTRAM_LSB, addr & 0xff);

    while(len-- & (addr++ < 0xfff))
        DS17485_REG_WRITE(dev->base_addr, DS17485_EXTRAM_DATA, *(buffer++));
}


u8 ds17485_get_model_number(const dev_t * const dev)
{
    /* Switch to the extended register set in order to read the model number */
    DS17485_SELECT_EXT_REG(dev->base_addr);

    return DS17485_REG_READ(dev->base_addr, DS17485_MODEL_NUMBER);
}


void ds17485_get_serial_number(const dev_t * const dev, u8 sn[6])
{
    /* Switch to the extended register set in order to read the serial number */
    DS17485_SELECT_EXT_REG(dev->base_addr);

    sn[0] = DS17485_REG_READ(dev->base_addr, DS17485_SERIAL_NUM_1);
    sn[1] = DS17485_REG_READ(dev->base_addr, DS17485_SERIAL_NUM_2);
    sn[2] = DS17485_REG_READ(dev->base_addr, DS17485_SERIAL_NUM_3);
    sn[3] = DS17485_REG_READ(dev->base_addr, DS17485_SERIAL_NUM_4);
    sn[4] = DS17485_REG_READ(dev->base_addr, DS17485_SERIAL_NUM_5);
    sn[5] = DS17485_REG_READ(dev->base_addr, DS17485_SERIAL_NUM_6);
}
