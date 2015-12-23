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


typedef enum irq_edge
{
	irq_edge_falling	= 0,
	irq_edge_rising		= 1
} irq_edge_t;


typedef enum irq
{
	irq_clk_a		= 0,
	irq_clk_b		= 1
} irq_t;


/* FIFO buffer structure - used for buffering received PS/2 data */
typedef struct fifo
{
	u8				rd;
	u8				wr;
	u8				data[256];	/* Hardwired to 256 to simplify buffer-wrapping calcs */
} fifo_t;


/* PS/2 channel context */
typedef struct ctx
{
	state_t			state;

	vu8 *			port;
	vu8 *			pin;
	vu8 *			ddr;
	u8				data_pin;
	u8				data;
	u8				parity_calc;
	u8				parity_received;
	u8				data_regnum;
	u8				command;
	u8				command_pending;
	void			(*start_tx_fn)(void);
	
	vu8 *			reg_status;
	vu8 *			reg_int_cfg;
	
	irq_t			irq;
	fifo_t			fifo;
} ctx_t;

#endif