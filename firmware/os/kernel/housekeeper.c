/*
    Housekeeper process - runs on a timer interrupt, performs background tasks.

    Part of ayumos


    (c) Stuart Wallace, November 2015.
*/


#include <kernel/device/device.h>
#include <kernel/housekeeper.h>
#include <kernel/include/process.h>
#include <kernel/util/kutil.h>


extern time_t g_current_timestamp;


/*
    housekeeper() - kernel process which performs various periodic tasks.
*/
void housekeeper(void *arg)
{
    u32 i;
    dev_t *rtc;
    rtc_time_t tm;
    u32 one = 1;
    UNUSED(arg);

    rtc = dev_find("rtc0");

    for(i = 0; 1; ++i)
    {
        if(!(i & 31))
        {
            /* Update current timestamp */
            if(rtc && (rtc->read(rtc, 0, &one, &tm) == SUCCESS))
                rtc_time_to_timestamp(&tm, &g_current_timestamp);
        }

        cpu_switch_process();
    }
}
