/*
    Dallas DS17485 driver function definitions

    Part of the as-yet-unnamed MC68010 operating system


    (c) Stuart Wallace, June 2015.
*/

#include <driver/ds17485.h>
#include <kernel/cpu.h>
#include <kernel/timer.h>
#include <kernel/util/kutil.h>


/* DS17485 state structure - stored in the base device's "data" member */
typedef struct ds17485_state
{
    tick_handler_fn_t   tick_fn;
} ds17485_state_t;


typedef struct ds17485_timer_setting
{
    u32 period_ns;
    u16 freq_hz;
    u8  bits;
} ds17485_timer_setting_t;

/* Mapping of period, frequency and DS17485 reg A setting for the programmable square-wave output */
ds17485_timer_setting_t g_ds17485_timer_settings[] =
{
/*   period/ns    freq/hz       RSA bits */
    {   122070,     8192,       0x03},
    {   244141,     4096,       0x04},
    {   488281,     2048,       0x05},
    {   976563,     1024,       0x06},
    {  1953125,      512,       0x07},
    {  3906250,      256,       0x00},
    {  7812500,      128,       0x01},
    { 15625000,       64,       0x0a},
    { 31250000,       32,       0x0b},
    { 62500000,       16,       0x0c},
    {125000000,        8,       0x0d},
    {250000000,        4,       0x0e},
    {500000000,        2,       0x0f},
    {        0,        0,       0x00}   /* Timer disabled */
};

/* Array mapping the lower four bits of DS17485 register A to square-wave output frequency */
const u16 g_ds17485_timer_freqs[] =
    {0, 256, 128, 8192, 4096, 2048, 1024, 512, 256, 128, 64, 32, 16, 8, 4, 2};


s32 ds17485_timer_control(dev_t *dev, const devctl_fn_t fn, const void *in, void *out);
s32 ds17485_timer_set_freq(dev_t *dev, ku32 freq, u32 *actual_freq);
s32 ds17485_timer_get_freq(dev_t *dev, u32 *freq);
void ds17485_default_tick_handler();


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
    ds17485_timer_init() - device initialiser for the periodic interrupt generator in a DS17485.
*/
s32 ds17485_timer_init(dev_t * const dev)
{
    dev->control = ds17485_timer_control;
    dev->len = 0;
    dev->block_size = 0;

    dev->data = dev->parent->data;

    return SUCCESS;
}


/*
    ds17485_init() - initialise a DS17485 RTC.
*/
s32 ds17485_init(dev_t * const dev)
{
    ds17485_state_t *state;
    void * const base_addr = dev->base_addr;

    state = kmalloc(sizeof(ds17485_state_t));
    if(!state)
        return ENOMEM;

    state->tick_fn = ds17485_default_tick_handler;
    dev->data = state;

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

    /* From this point forward, we are no longer in interrupt context. */
    if(intflags & DS17485_PF)
    {
        /* Periodic interrupt */
        ((ds17485_state_t *) dev->data)->tick_fn();
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


/*
    ds17485_timer_set_freq() - set the frequency of the DS17485 timer
*/
s32 ds17485_timer_set_freq(dev_t *dev, ku32 freq, u32 *actual_freq)
{
    ds17485_timer_setting_t *setting;
    const void * const base_addr = dev->base_addr;
    u8 ra;

    /*
        Find the nearest frequency to the requested one.  Note that if the requested frequency is
        not available, the next-lowest valid frequency will be chosen.
    */
    FOR_EACH(setting, g_ds17485_timer_settings)
    {
        if(setting->freq_hz <= freq)
            break;
    }

    /* Set the square wave frequency */
    ra = DS17485_REG_READ(base_addr, DS17485_REG_A);

    ra &= ~DS17485_REG_A_RS_MASK;
    ra |= setting->bits;

    DS17485_REG_WRITE(base_addr, DS17485_REG_A, ra);

    if(actual_freq)
        *actual_freq = setting->freq_hz;

    return SUCCESS;
}


/*
    ds17485_timer_get_freq() - get the frequency of the DS17485 timer
*/
s32 ds17485_timer_get_freq(dev_t *dev, u32 *freq)
{
    ku8 ra = DS17485_REG_READ(dev->base_addr, DS17485_REG_A) & DS17485_REG_A_RS_MASK;

    *freq = (u32) g_ds17485_timer_freqs[ra];

    return SUCCESS;
}


/*
    ds17485_timer_control() - devctl responder for the DS17485 timer device.
*/
s32 ds17485_timer_control(dev_t *dev, const devctl_fn_t fn, const void *in, void *out)
{
    const void * const base_addr = dev->base_addr;
    ds17485_state_t *state = (ds17485_state_t *) dev->data;
    ku32 u32_in = *((u32 *) in);

    switch(fn)
    {
        case dc_timer_set_freq:
            return ds17485_timer_set_freq(dev, u32_in, (u32 *) out);

        case dc_timer_get_freq:
            return ds17485_timer_get_freq(dev, (u32 *) out);

        case dc_timer_set_enable:
            if(u32_in)
                DS17485_REG_SET_BITS(base_addr, DS17485_REG_B, DS17485_PIE);
            else
                DS17485_REG_CLEAR_BITS(base_addr, DS17485_REG_B, DS17485_PIE);

            return SUCCESS;

        case dc_timer_get_enable:
            *((u32 *) out) = (DS17485_REG_READ(base_addr, DS17485_REG_B) & DS17485_PIE) ? 1 : 0;
            return SUCCESS;

        case dc_timer_set_tick_fn:
            state->tick_fn = u32_in ? (tick_handler_fn_t) in : ds17485_default_tick_handler;
            return SUCCESS;

        default:
            return ENOSYS;
    }
}


void ds17485_default_tick_handler()
{
    /* Do nothing */
}
