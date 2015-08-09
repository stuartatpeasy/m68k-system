#ifndef DEVICE_LED_H_INC
#define DEVICE_LED_H_INC
/*
    Motherboard LED control functions

    Part of the as-yet-unnamed MC68010 operating system


    (c) Stuart Wallace, July 2015
*/

#include "device/duart.h"
#include "include/defs.h"
#include "include/types.h"

#define LED_ON_PORT                 (DUART_ROPR)
#define LED_OFF_PORT                (DUART_SOPR)

/* LED port bits */
#define LED_GREEN					(0x80)
#define LED_RED						(0x40)


void led_on(const unsigned char leds);
void led_off(const unsigned char leds);

#endif
