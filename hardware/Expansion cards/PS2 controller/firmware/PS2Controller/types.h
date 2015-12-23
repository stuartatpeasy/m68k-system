#ifndef TYPES_H_
#define TYPES_H_
/*
	types.h - Custom type definitions for the dual-channel PS/2 controller

	This module contains firmware for an m68k-bus peripheral controller.


	(c) Stuart Wallace <stuartw@atom.net>, November 2015.
*/

typedef unsigned char	u8;
typedef const u8		ku8;
typedef volatile u8		vu8;


/* PS/2 data transmission/reception states */
typedef enum state
{
	state_idle		= 0,
	state_rx_start	= 1,
	state_rx_d0		= 2,
	state_rx_d1		= 3,
	state_rx_d2		= 4,
	state_rx_d3		= 5,
	state_rx_d4		= 6,
	state_rx_d5		= 7,
	state_rx_d6		= 8,
	state_rx_d7		= 9,
	state_rx_parity	= 10,
	state_rx_stop	= 11,
	
	state_tx_busrq	= 12,
	state_tx_start	= 13,
	state_tx_d0		= 14,
	state_tx_d1		= 15,
	state_tx_d2		= 16,
	state_tx_d3		= 17,
	state_tx_d4		= 18,
	state_tx_d5		= 19,
	state_tx_d6		= 20,
	state_tx_d7		= 21,
	state_tx_parity	= 22,
	state_tx_stop	= 23,
	state_tx_ack	= 24,
} state_t;


/* FIFO buffer structure - used for buffering received PS/2 data */
typedef struct fifo
{
	u8				rd;
	u8				wr;
	u8				data[256];	/* Hardwired to 256 to simplify buffer-wrapping calcs */
} fifo_t;


/* Timer events - specifies the duration of a timer, and the action to be taken on timeout */
typedef enum timer_event
{
	timer_event_none = 0,
	timer_event_tx_rq,
	timer_event_tx_clock_start,
	timer_event_rx_bit_wait,
	timer_event_tx_bit_wait,
	timer_event_cmd_response
} timer_event_t;


/* PS/2 channel context */
typedef struct ctx
{
	state_t			state;
	
	struct
	{
		struct
		{
			vu8 *	port;
			vu8 *	pin;
			vu8 *	ddr;
			u8		data_pin;
			u8		clk_pin;
		} io;
		
		struct  
		{
			vu8				*count_reg;
			vu8				*clk_reg;
			u8				irq_enable_bit;
			timer_event_t	event;
		} timer;
	} uc_regs;
	
	struct
	{
		vu8 *		status;
		vu8 *		int_cfg;
	} host_regs;

	u8				data;
	u8				parity_calc;
	u8				parity_received;
	u8				command;
	u8				command_pending;

	u8				irq_enable_bit;
	fifo_t			fifo;
} ctx_t;

#endif