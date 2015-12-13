/*
	PS2Controller.c - PS/2 keyboard & mouse controller implementation
	
	This module contains firmware for an m68k-bus peripheral controller.
	
	
	(c) Stuart Wallace <stuartw@atom.net>, November 2015.
 */

#ifndef PS2CONTROLLER_H_
#define PS2CONTROLLER_H_

#define F_CPU		8000000UL		/* 8MHz CPU clock */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "debug.h"
#include "macros.h"
#include "portdefs.h"
#include "registers.h"

#define PERIPHERAL_ID		(0x82)		/* Peripheral identifier */

#define IRQ_EDGE_FALLING	(0)
#define IRQ_EDGE_RISING		(1)

typedef unsigned char	u8;
typedef const u8		ku8;

/* PS/2 data transmission/reception states */
typedef enum data_state
{
	ds_idle			= 0,
	ds_rx_start		= 1,
	ds_rx_d0		= 2,
	ds_rx_d1		= 3,
	ds_rx_d2		= 4,
	ds_rx_d3		= 5,
	ds_rx_d4		= 6,
	ds_rx_d5		= 7,
	ds_rx_d6		= 8,
	ds_rx_d7		= 9,
	ds_rx_parity	= 10,
	ds_rx_stop		= 11,
	
	ds_tx_busrq		= 12,
	dx_tx_start		= 13,
} data_state_t;


/* PS/2 link states */
typedef enum cmd_state
{
	cs_rx_data		= 0,		/* Waiting to receive, or receiving, scan codes (normal state)	*/
	cs_ignore		= 1,		/* Ignoring junk sent by the device after power-up				*/
	cs_cmd_pending	= 2,
	cs_ack_wait		= 3,		/* Waiting for an acknowledgment for a previous command			*/
} cmd_state_t;


typedef enum irq_edge
{
	irq_edge_falling	= 0,
	irq_edge_rising		= 1
} irq_edge_t;

typedef enum irq
{
	irq_keyboard_clk	= 0,
	irq_mouse_clk		= 1
} irq_t;


#define CMD_NONE			(0x00)
#define KB_CMD_SET_LEDS		(0xed)
#define KB_CMD_SET_TM_RATE	(0xf3)
#define KB_CMD_RESET		(0xff)


/* Keyboard/mouse data transmission/reception context */
typedef struct ctx
{
	cmd_state_t		state_cmd;
	data_state_t	state_data;

	u8				data;
	u8				parity_calc;
	u8				parity_received;
	u8				data_regnum;
	u8				command;
} ctx_t;


void init(void);
void set_irq_edge(const irq_t irq, const irq_edge_t edge);
void host_reg_write(ku8 reg, const u8 data);
void process_clock_edge(ctx_t *ctx, u8 bit);


#endif