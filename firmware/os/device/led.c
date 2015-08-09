/*
    Motherboard LED control functions

    Part of the as-yet-unnamed MC68010 operating system


    (c) Stuart Wallace, July 2015
*/

#include "led.h"


/*
	led_on() - switch on the red and/or green LEDs
*/
void led_on(const unsigned char leds)
{
	LED_ON_PORT = leds & (LED_RED | LED_GREEN);
}


/*
	led_off() - switch off the red and/or green LEDs
*/
void led_off(const unsigned char leds)
{
	LED_OFF_PORT = leds & (LED_RED | LED_GREEN);
}
