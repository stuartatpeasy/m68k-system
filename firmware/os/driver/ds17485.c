/*
    Dallas DS17485 driver function definitions

    Part of the as-yet-unnamed MC68010 operating system


    (c) Stuart Wallace, June 2015.
*/

#include <driver/ds17485.h>
#include <kernel/cpu.h>
#include <kernel/util/kutil.h>


/*
    ds17485_rtc_init() - device initialiser for the RTC in a DS17485.
*/
s32 ds17485_rtc_init(dev_t * const dev)
{
    void * const base_addr = dev->base_addr;
    u8 reg_a_val;

    dev->read = ds17485_rtc_read;
    dev->write = ds17485_rtc_write;
    dev->len = sizeof(rtc_time_t);
    dev->block_size = 1;

    dev->data = dev->parent->data;

    /* Configure and enable 2Hz periodic interrupt */
    cpu_irq_add_handler(dev->irql, dev, ds17485_irq);

    reg_a_val = DS17485_REG_READ(base_addr, DS17485_REG_A) & ~DS17485_REG_A_RS_MASK;
    reg_a_val |= DS17485_SQW_2HZ << DS17485_REG_A_RS_SHIFT;
    DS17485_REG_WRITE(base_addr, DS17485_REG_A, reg_a_val);

    DS17485_REG_SET_BITS(base_addr, DS17485_REG_B, DS17485_PIE);

    return SUCCESS;
}


/*
    ds17485_user_ram_init() - device initialiser for the user NVRAM in a DS17485.
*/
s32 ds17485_user_ram_init(dev_t * const dev)
{
    dev->read = ds17485_user_ram_read;
    dev->write = ds17485_user_ram_write;
    dev->len = DS17485_USER_RAM_LEN;
    dev->block_size = 1;

    dev->data = dev->parent->data;

    return SUCCESS;
}


/*
    ds17485_ext_ram_init() - device initialiser for the extended NVRAM in a DS17485.
*/
s32 ds17485_ext_ram_init(dev_t * const dev)
{
    dev->read = ds17485_ext_ram_read;
    dev->write = ds17485_ext_ram_write;
    dev->len = DS17485_EXT_RAM_LEN;
    dev->block_size = 1;

    dev->data = dev->parent->data;

    return SUCCESS;
}


/*
    ds17485_init() - initialise a DS17485 RTC.
*/
s32 ds17485_init(dev_t * const dev)
{
    void * const base_addr = dev->base_addr;

    /*
        Write register A:
            UIP      = 0    (ignored; read only)
            DV2      = 0    Enable countdown chain
            DV1      = 1    Oscillator on, VCC power-up state
            DV0      = 0    Select standard register bank
            RS[3..0] = [x]  Set rate-select bits for 2Hz square wave / periodic interrupt
    */
    DS17485_REG_WRITE(base_addr, DS17485_REG_A, DS17485_DV1);

    /* Set up extended registers */
    DS17485_SELECT_EXT_REG(base_addr);

    /* Write register 4A: enable extended RAM burst mode */
    DS17485_REG_WRITE(base_addr, DS17485_REG_4A, DS17485_BME);

    /* Write register 4B: enable RAM clear input pin; select 12.5pF crystal load capacitance */
    DS17485_REG_WRITE(base_addr, DS17485_REG_4B, DS17485_RCE | DS17485_CS);

    DS17485_SELECT_STD_REG(base_addr);  /* Switch back to standard registers */

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
    DS17485_REG_WRITE(base_addr, DS17485_REG_B, DS17485_DM | DS17485_2412);

    ds17485_force_valid_time(dev);

    return SUCCESS;
}


/*
    ds17485_irq() - interrupt service routine
*/
void ds17485_irq(ku32 irql, void *data)
{
    dev_t * const dev = (dev_t *) data;
    void * const base_addr = dev->base_addr;
    UNUSED(irql);

    /* Read register C to clear the interrupt conditions */
    ku8 intflags = DS17485_REG_READ(base_addr, DS17485_REG_C);

    if(intflags & DS17485_PF)
    {
        /* Periodic interrupt */
    }

    if(intflags & DS17485_AF)
    {
        /* Alarm interrupt */
    }
}


/*
    ds17485_rtc_read() - read the current time and populate a struct rtc_time_t.
*/
s32 ds17485_rtc_read(dev_t * const dev, ku32 offset, u32 *len, void *buffer)
{
    void * const base_addr = dev->base_addr;
    rtc_time_t * const tm = (rtc_time_t *) buffer;

    if(offset || (*len != 1))
        return EINVAL;

    /* Set the "SET" bit in register B, to prevent updates while we read */
    DS17485_REG_SET_BITS(base_addr, DS17485_REG_B, DS17485_SET);

    /* Need to switch to extended register set in order to read the century */
    DS17485_SELECT_EXT_REG(base_addr);

    tm->year = 100 * DS17485_REG_READ(base_addr, DS17485_CENTURY);

    /* Switch back to the standard register set to read the rest of the date/time */
    DS17485_SELECT_STD_REG(base_addr);

    tm->hour = DS17485_REG_READ(base_addr, DS17485_HOURS);
    tm->minute = DS17485_REG_READ(base_addr, DS17485_MINUTES);
    tm->second = DS17485_REG_READ(base_addr, DS17485_SECONDS);

    tm->day_of_week = DS17485_REG_READ(base_addr, DS17485_DAY_OF_WEEK);
    tm->day = DS17485_REG_READ(base_addr, DS17485_DAY);
    tm->month = DS17485_REG_READ(base_addr, DS17485_MONTH);
    tm->year += DS17485_REG_READ(base_addr, DS17485_YEAR);

    tm->dst = DS17485_REG_READ(base_addr, DS17485_REG_B) & DS17485_DSE;

    /* Clear the "SET" bit in register B, as we have finished reading data */
    DS17485_REG_CLEAR_BITS(base_addr, DS17485_REG_B, DS17485_SET);

    return SUCCESS;
}


/*
    ds17485_rtc_write() - set the time stored in a DS17485 to the time specified by tm.
*/
s32 ds17485_rtc_write(dev_t * const dev, ku32 offset, u32 *len, const void *buffer)
{
    void * const base_addr = dev->base_addr;
    const rtc_time_t * const tm = (const rtc_time_t *) buffer;

    if(offset || (*len != 1))
        return EINVAL;

    /* Set the "SET" bit in register B, to prevent updates while we write */
    DS17485_REG_SET_BITS(base_addr, DS17485_REG_B, DS17485_SET);

    /* Need to switch to extended register set in order to write the century */
    DS17485_SELECT_EXT_REG(base_addr);

    DS17485_REG_WRITE(base_addr, DS17485_CENTURY, tm->year / 100);

    /* Switch back to the standard register set to read the rest of the date/time */
    DS17485_SELECT_STD_REG(base_addr);

    DS17485_REG_WRITE(base_addr, DS17485_YEAR, tm->year % 100);

    DS17485_REG_WRITE(base_addr, DS17485_MONTH, tm->month);
    DS17485_REG_WRITE(base_addr, DS17485_DAY, tm->day);
    DS17485_REG_WRITE(base_addr, DS17485_HOURS, tm->hour);
    DS17485_REG_WRITE(base_addr, DS17485_MINUTES, tm->minute);
    DS17485_REG_WRITE(base_addr, DS17485_SECONDS, tm->second);

    DS17485_REG_WRITE(base_addr, DS17485_DAY_OF_WEEK, tm->day_of_week);

    DS17485_REG_SET_BITS(base_addr, DS17485_REG_B, (tm->dst > 0));

    /* Clear the "SET" bit in register B, as we have finished writing data */
    DS17485_REG_CLEAR_BITS(base_addr, DS17485_REG_B, DS17485_SET);

    return SUCCESS;
}


void ds17485_force_valid_time(dev_t * const dev)
{
    rtc_time_t ts;
    u32 one = 1;

    ds17485_rtc_read(dev, 0, &one, &ts);

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

        ds17485_rtc_write(dev, 0, &one, &ts);
    }
}


/*
    ds17485_user_ram_read() - read len bytes of user NVRAM from offset addr into buffer
*/
s32 ds17485_user_ram_read(dev_t * const dev, u32 addr, u32 *len, void * buffer)
{
    const void * const base_addr = dev->base_addr;
    u32 len_ = *len;

    if((addr + len_) > DS17485_USER_RAM_LEN)
        return EINVAL;

    DS17485_SELECT_STD_REG(base_addr);
    for(addr += DS17485_USER_RAM_START; len_; --len_, ++addr)
        *((u8 *) buffer++) = DS17485_REG_READ(base_addr, addr);

    return SUCCESS;
}


/*
    ds17485_user_ram_write() - write len bytes from buffer into user NVRAM at offset addr
*/
s32 ds17485_user_ram_write(dev_t * const dev, u32 addr, u32 *len, const void * buffer)
{
    const void * const base_addr = dev->base_addr;
    u32 len_ = *len;

    if((addr + len_) > DS17485_USER_RAM_LEN)
        return EINVAL;

    DS17485_SELECT_STD_REG(base_addr);
    for(addr += 14; len_ && (addr < 128); --len_, ++addr)
        DS17485_REG_WRITE(base_addr, addr, *((u8 *) buffer++));

    return SUCCESS;
}


/*
    ds17485_ext_ram_read() - read len bytes of extended NVRAM from offset addr into buffer
*/
s32 ds17485_ext_ram_read(dev_t * const dev, u32 addr, u32 *len, void * buffer)
{
    const void * const base_addr = dev->base_addr;
    u32 len_ = *len;

    if((addr + len_) > DS17485_EXT_RAM_LEN)
        return EINVAL;

    /* Switch to the extended register set in order to read the extended RAM area */
    DS17485_SELECT_EXT_REG(base_addr);

    DS17485_REG_WRITE(base_addr, DS17485_EXTRAM_MSB, (addr & 0xf00) >> 8);
    DS17485_REG_WRITE(base_addr, DS17485_EXTRAM_LSB, addr & 0xff);

    while(len_-- & (addr++ < 0xfff))
        *((u8 *) buffer++) = DS17485_REG_READ(base_addr, DS17485_EXTRAM_DATA);

    return SUCCESS;
}


/*
    ds17485_ext_ram_write() - write len bytes from buffer into extended NVRAM at offset addr
*/
s32 ds17485_ext_ram_write(dev_t * const dev, u32 addr, u32 *len, const void * buffer)
{
    const void * const base_addr = dev->base_addr;
    u32 len_ = *len;

    if((addr + len_) > DS17485_EXT_RAM_LEN)
        return EINVAL;

    /* Switch to the extended register set in order to read the extended RAM area */
    DS17485_SELECT_EXT_REG(base_addr);

    DS17485_REG_WRITE(base_addr, DS17485_EXTRAM_MSB, (addr & 0xf00) >> 8);
    DS17485_REG_WRITE(base_addr, DS17485_EXTRAM_LSB, addr & 0xff);

    while(len_-- & (addr++ < 0xfff))
        DS17485_REG_WRITE(base_addr, DS17485_EXTRAM_DATA, *((u8 *) buffer++));

    return SUCCESS;
}


/*
    ds17485_get_model_number() - get the eight-bit model number for this RTC
*/
u8 ds17485_get_model_number(const dev_t * const dev)
{
    const void * const base_addr = dev->base_addr;

    /* Switch to the extended register set in order to read the model number */
    DS17485_SELECT_EXT_REG(base_addr);

    return DS17485_REG_READ(base_addr, DS17485_MODEL_NUMBER);
}


/*
    ds17485_get_serial_number() - get the hardware serial number from the RTC
*/
void ds17485_get_serial_number(const dev_t * const dev, u8 sn[6])
{
    const void * const base_addr = dev->base_addr;

    /* Switch to the extended register set in order to read the serial number */
    DS17485_SELECT_EXT_REG(base_addr);

    sn[0] = DS17485_REG_READ(base_addr, DS17485_SERIAL_NUM_1);
    sn[1] = DS17485_REG_READ(base_addr, DS17485_SERIAL_NUM_2);
    sn[2] = DS17485_REG_READ(base_addr, DS17485_SERIAL_NUM_3);
    sn[3] = DS17485_REG_READ(base_addr, DS17485_SERIAL_NUM_4);
    sn[4] = DS17485_REG_READ(base_addr, DS17485_SERIAL_NUM_5);
    sn[5] = DS17485_REG_READ(base_addr, DS17485_SERIAL_NUM_6);
}
