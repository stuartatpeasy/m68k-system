/*
	MC68681 DUART "driver"

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace, December 2011.
*/
#include "device/duart.h"


int duarta_putc(const char c)
{
	while(!(DUART_SRA & (1 << DUART_SR_TXEMT))) ;
	DUART_THRA = c;
	return c;
}


int duarta_getc(void)
{
	while(!(DUART_SRA & (1 << DUART_SR_RXRDY))) ;
	return DUART_RHRA;
}


/*
    duart_start_counter() - start the DUART counter
*/
void duart_start_counter(void)
{
    u8 dummy;

    /* Set CTUR/CTLR - the counter/timer upper/lower timeout counts */
    DUART_CTUR = (((DUART_CLK_HZ / 16) / TICK_RATE) & 0xff00) >> 8;
    DUART_CTLR = ((DUART_CLK_HZ / 16) / TICK_RATE) & 0xff;

    dummy = DUART_START_CC;
    dummy += 0;     /* silence the "var set but not used" compiler warning */
}


/*
    duart_stop_counter() - stop the DUART counter
    Check the MC68681 data sheet - there may be some oddness related to this.
*/
void duart_stop_counter(void)
{
    u8 dummy = DUART_STOP_CC;
    dummy += 0;     /* silence the "var set but not used" compiler warning */
}
