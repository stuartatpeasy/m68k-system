/*
	PS2Controller.c - dual-channel PS/2 controller implementation
	
	This module contains firmware for an m68k-bus peripheral controller.
	
	
	(c) Stuart Wallace <stuartw@atom.net>, November 2015.
 */

//#define WITH_DEBUGGING				/* Enable debugging */

#include "PS2Controller.h"

vu8 host_regs[8];
volatile ctx_t
	ctx_a = {
		.state					= state_idle,
		.command				= 0,
		.command_pending		= 0,
		.irq_enable_bit			= _BV(INT0),

		.uc_regs = {
			.io = {
				.port			= &CHAN_A_PORT,
				.pin			= &CHAN_A_PIN,
				.ddr			= &CHAN_A_DDR,
				.data_pin		= CHAN_A_DATA,
				.clk_pin		= CHAN_A_CLK
			},
			.timer = {
				.count_reg		= &TCNT0,
				.clk_reg		= &TCCR0,
				.irq_enable_bit	= _BV(TOIE0)
			}
		},

		.host_regs = {
			.status				= &host_regs[REG_STATUS_A],
			.int_cfg			= &host_regs[REG_INT_CFG_A]
		},
	
		.fifo = {
			.rd					= 0,
			.wr					= 0
		}
	},

	ctx_b = {
		.state					= state_idle,
		.command				= 0,
		.command_pending		= 0,
		.irq_enable_bit			= _BV(INT1),

		.uc_regs = {
			.io = {
				.port			= &CHAN_B_PORT,
				.pin			= &CHAN_B_PIN,
				.ddr			= &CHAN_B_DDR,
				.data_pin		= CHAN_B_DATA,
				.clk_pin		= CHAN_B_CLK
			},
			.timer = {
				.count_reg		= &TCNT2,
				.clk_reg		= &TCCR2,
				.irq_enable_bit	= _BV(TOIE2)
			}
		},

		.host_regs = {
			.status				= &host_regs[REG_STATUS_B],
			.int_cfg			= &host_regs[REG_INT_CFG_B]
		},
		
		.fifo = {
			.rd					= 0,
			.wr					= 0
		}
	};


/*
	ISR for INT0 - called on falling edges of channel A clock; handles incoming data.
*/
ISR(INT0_vect)
{
	process_clock_edge(&ctx_a);
}


/*
	ISR for INT1 - called on falling edges of channel B clock; handles incoming data.
*/
ISR(INT1_vect)
{
	process_clock_edge(&ctx_b);
}


/*
	ISR for timer 0 overflow - handles channel A events.
*/
ISR(TIMER0_OVF_vect)
{
	handle_timer_event(&ctx_a);
}


/*
	ISR for timer 2 overflow - handles channel B events.
*/
ISR(TIMER2_OVF_vect)
{
	handle_timer_event(&ctx_b);
}


/*
	init() - initialise the chip following a reset
*/
void init(void)
{
	cli();

	/* Configure ports */
	DDRD			= PORTD_OUTPUTS;
	PORTD			= PORTD_PULLUPS;

	DDRC			= PORTC_OUTPUTS;
	PORTC			= PORTC_PULLUPS;

	DATA_BUS_DDR	= DATA_BUS_OUTPUTS;
	DATA_BUS_PORT	= DATA_BUS_PULLUPS;

	SET_HIGH(PORTD, nACK);

	/* Disable timers */
	TCCR0	= 0;
	TCCR1A	= 0;
	TCCR1B	= 0;
	TCCR2	= 0;

	/* Initially disable power to both PS/2 channels */
	SET_HIGH(PWR_PORT, nPWR_A | nPWR_B);

	/* Enable interrupts on falling edge of PS/2 clock inputs */
	MCUCR = (MCUCR | _BV(ISC01)) & ~_BV(ISC00);
	MCUCR = (MCUCR | _BV(ISC11)) & ~_BV(ISC10);

	SET_HIGH(GICR, _BV(INT0) | _BV(INT1));

	debug_init();

	sei();
}


/*
	process_data() - process a byte of data received from a PS/2 channel.
*/
void process_data(volatile ctx_t *ctx)
{
	if(ctx->parity_received == ctx->parity_calc)
	{
		ku8 data = ctx->data,
			fifo_count = ctx->fifo.wr - ctx->fifo.rd;

		/* Place received data in FIFO */
		ctx->fifo.data[ctx->fifo.wr++] = data;
		*ctx->host_regs.status |= FLAG_RX;
		HOST_IRQ_ASSERT_IF(*ctx->host_regs.int_cfg & FLAG_RX);

		if(fifo_count > FIFO_HIGH_WATER_MARK)
		{
			/* Inhibit communication until the FIFO is drained by the host */
			SET_LOW(*ctx->uc_regs.io.port, ctx->uc_regs.io.clk_pin);
			SET_OUTPUT(*ctx->uc_regs.io.ddr, ctx->uc_regs.io.clk_pin);
		}

		debug_puthexb(data);
		debug_putc('\n');
	}
	else
	{
		/* Parity error */
		*ctx->host_regs.status |= FLAG_PAR_ERR;
		HOST_IRQ_ASSERT_IF(*ctx->host_regs.int_cfg & FLAG_PAR_ERR);

		debug_putc('!');
	}
}


/*
	process_clock_edge() - handle a clock transition on a PS/2 port.
	This function deals with the transmission and receipt of data on the ports.
*/
void process_clock_edge(volatile ctx_t *ctx)
{
	ku8 data_pin = ctx->uc_regs.io.data_pin;

	switch(++(ctx->state))
	{
		case state_idle:		/* Unreachable					*/
		case state_tx_busrq:	/* Unreachable (handled in ISR)	*/
			break;

		case state_rx_start:
			ctx->parity_calc = 1;		/* because odd parity */
			ctx->uc_regs.timer.event = timer_event_rx_bit_wait;
			timer_start(ctx);
			break;

		case state_rx_d0:
		case state_rx_d1:
		case state_rx_d2:
		case state_rx_d3:
		case state_rx_d4:
		case state_rx_d5:
		case state_rx_d6:
		case state_rx_d7:
			ctx->data >>= 1;
			if(*ctx->uc_regs.io.pin & data_pin)
			{
				ctx->data |= 0x80;
				ctx->parity_calc ^= 1;
			}
			ctx->uc_regs.timer.event = timer_event_rx_bit_wait;
			timer_start(ctx);
			break;

		case state_rx_parity:
			ctx->parity_received = (*ctx->uc_regs.io.pin & data_pin) ? 1 : 0;
			ctx->uc_regs.timer.event = timer_event_rx_bit_wait;
			timer_start(ctx);
			break;

		case state_rx_stop:
			process_data(ctx);
			if(ctx->command_pending)
				start_tx(ctx);
			else
				ctx->state = state_idle;
			timer_stop(ctx);
			break;

		case state_tx_start:
			SET_LOW(*ctx->uc_regs.io.port, data_pin);
			ctx->parity_calc = 1;		/* because odd parity */
			ctx->uc_regs.timer.event = timer_event_tx_bit_wait;
			timer_start(ctx);
			break;

		case state_tx_d0:
		case state_tx_d1:
		case state_tx_d2:
		case state_tx_d3:
		case state_tx_d4:
		case state_tx_d5:
		case state_tx_d6:
		case state_tx_d7:
			if(ctx->command & 0x1)
			{
				SET_HIGH(*ctx->uc_regs.io.port, data_pin);
				ctx->parity_calc ^= 1;
			}
			else
				SET_LOW(*ctx->uc_regs.io.port, data_pin);

			ctx->command >>= 1;
			ctx->uc_regs.timer.event = timer_event_tx_bit_wait;
			timer_start(ctx);
			break;
		
		case state_tx_parity:
			if(ctx->parity_calc)
				SET_HIGH(*ctx->uc_regs.io.port, data_pin);
			else
				SET_LOW(*ctx->uc_regs.io.port, data_pin);
			ctx->uc_regs.timer.event = timer_event_tx_bit_wait;
			timer_start(ctx);
			break;

		case state_tx_stop:
			SET_HIGH(*ctx->uc_regs.io.port, data_pin);
			SET_INPUT(*ctx->uc_regs.io.ddr, data_pin);
			ctx->uc_regs.timer.event = timer_event_tx_bit_wait;
			timer_start(ctx);
			break;

		case state_tx_ack:
			*ctx->host_regs.status |= FLAG_TX;
			ctx->state = state_idle;
			HOST_IRQ_ASSERT_IF(*ctx->host_regs.int_cfg & FLAG_TX);
			timer_stop(ctx);

			if(ctx->command_pending)
				start_tx(ctx);
			break;
	}
}


/*
	handle_timer_event() - handle an overflow interrupt from a channel's timer.
*/
void handle_timer_event(volatile ctx_t *ctx)
{
	/* Disable timer */
	SET_LOW(TIMSK, ctx->uc_regs.timer.irq_enable_bit);
	SET_LOW(*ctx->uc_regs.timer.clk_reg, _BV(CS02) | _BV(CS01) | _BV(CS00));

	switch(ctx->uc_regs.timer.event)
	{
		case timer_event_tx_rq:
			/* Host has finished requesting attention from the device */
			SET_LOW(*ctx->uc_regs.io.port, ctx->uc_regs.io.data_pin);
			SET_OUTPUT(*ctx->uc_regs.io.ddr, ctx->uc_regs.io.data_pin);
			SET_HIGH(*ctx->uc_regs.io.port, ctx->uc_regs.io.clk_pin);
			SET_INPUT(*ctx->uc_regs.io.ddr, ctx->uc_regs.io.clk_pin);

			/* Re-enable channel B clock interrupt */
			SET_HIGH(GICR, ctx->irq_enable_bit);
			break;

		case timer_event_tx_clock_start:
			/* Device did not start generating clock pulses in a timely fashion */
			ctx->state = state_idle;
			*ctx->host_regs.status |= FLAG_START_TIMEOUT;
			HOST_IRQ_ASSERT_IF(*ctx->host_regs.int_cfg & FLAG_START_TIMEOUT);
			break;

		case timer_event_rx_bit_wait:
			/* Device did not send the next bit of data in a timely fashion */
			ctx->state = state_idle;
			*ctx->host_regs.status |= FLAG_RX_TIMEOUT;
			HOST_IRQ_ASSERT_IF(*ctx->host_regs.int_cfg & FLAG_RX_TIMEOUT);
			break;

		case timer_event_tx_bit_wait:
			/* Device did not receive the next bit of data in a timely fashion */
			ctx->state = state_idle;
			*ctx->host_regs.status |= FLAG_TX_TIMEOUT;
			HOST_IRQ_ASSERT_IF(*ctx->host_regs.int_cfg & FLAG_TX_TIMEOUT);
			break;

		case timer_event_cmd_response:
			/* Device did not respond to command transmission in a timely fashion */
			ctx->state = state_idle;
			*ctx->host_regs.status |= FLAG_CMD_TIMEOUT;
			HOST_IRQ_ASSERT_IF(*ctx->host_regs.int_cfg & FLAG_CMD_TIMEOUT);
			break;

		case timer_event_none:		/* Should never be reached */
			break;
	}
	ctx->uc_regs.timer.event = timer_event_none;
}


/*
	start_tx() - start transmitting a byte on one of the PS/2 channels.
*/
void start_tx(volatile ctx_t *ctx)
{
	SET_LOW(GICR, ctx->irq_enable_bit);
	ctx->command_pending = 0;

	/* Start channel A command transmission by pulling the PS/2 clock line low */
	SET_LOW(*ctx->uc_regs.io.port, ctx->uc_regs.io.clk_pin);
	SET_OUTPUT(*ctx->uc_regs.io.ddr, ctx->uc_regs.io.clk_pin);

	/* Generate an interrupt after 150us */
	ctx->uc_regs.timer.event = timer_event_tx_rq;
	timer_start(ctx);
	ctx->state = state_tx_busrq;
}


/*
	timer_start() - start a timer with a duration approriate for the specified event; cause an
	interrupt to be raised when the timer expires.
*/
void timer_start(volatile ctx_t *ctx)
{
	/* NOTE: the counter calculations in this function assume f(clk) = 8MHz! */
	switch(ctx->uc_regs.timer.event)
	{
		case timer_event_none:
			break;

		case timer_event_tx_rq:				/* 150us */
			/* overflow after 150 counts @ f(clk)/8  */
			*ctx->uc_regs.timer.count_reg = (u8) (256 - 150);
			*ctx->uc_regs.timer.clk_reg = _BV(CS01);
			break;

		case timer_event_rx_bit_wait:			/* 2ms */
		case timer_event_tx_bit_wait:			/* 2ms */
			/* overflow after 250 counts @ f(clk)/64 */
			*ctx->uc_regs.timer.count_reg = (u8) (256 - 250);
			*ctx->uc_regs.timer.clk_reg = _BV(CS01) | _BV(CS00);
			break;

		case timer_event_tx_clock_start:	/* 15ms */
			/* overflow after 118 counts @ f(clk)/1024 */
			*ctx->uc_regs.timer.count_reg = (u8) (256 - 118);
			*ctx->uc_regs.timer.clk_reg = _BV(CS02) | _BV(CS00);
			break;

		case timer_event_cmd_response:		/* 20ms */
			/* overflow after 157 counts @ f(clk)/1024 */
			*ctx->uc_regs.timer.count_reg = (u8) (256 - 157);
			*ctx->uc_regs.timer.clk_reg = _BV(CS02) | _BV(CS00);
			break;
	}
	SET_HIGH(TIMSK, ctx->uc_regs.timer.irq_enable_bit);
}


/*
	timer_stop() - stop the timer started with start_timer().
*/
void timer_stop(volatile ctx_t *ctx)
{
	SET_LOW(TIMSK, ctx->uc_regs.timer.irq_enable_bit);
	SET_LOW(*ctx->uc_regs.timer.clk_reg, _BV(CS02) | _BV(CS01) | _BV(CS00));
}


/*
	fifo_read() - try to read a byte from a FIFO.
*/
inline void fifo_read(volatile ctx_t *ctx)
{
	ku8 fifo_count = ctx->fifo.wr - ctx->fifo.rd;

	if(!fifo_count)
	{
		DO_READ_CYCLE(0);		/* FIFO is empty */
		*ctx->host_regs.status &= ~FLAG_RX;
	}
	else
	{
		DO_READ_CYCLE(ctx->fifo.data[ctx->fifo.rd++]);

		if(fifo_count < FIFO_LOW_WATER_MARK)
		{
			/* Enable reception */
			SET_HIGH(*ctx->uc_regs.io.port, ctx->uc_regs.io.clk_pin);
			SET_INPUT(*ctx->uc_regs.io.ddr, ctx->uc_regs.io.clk_pin);

			if(fifo_count == 1)
				*ctx->host_regs.status &= ~FLAG_RX;
		}
	}
}


/*
	try_start_tx() - try to start transmitting a command; if the channel is busy, set a flag
	indicating that a transmission is pending.
*/
inline void try_start_tx(volatile ctx_t *ctx, ku8 data)
{
	ctx->command = data;
	*ctx->host_regs.status &= ~FLAG_TX;

	if(ctx->state == state_idle)
		start_tx(ctx);
	else
		ctx->command_pending = 1;
}


/*
	main() - main loop: manage communications with the host.
*/
int main(void)
{
	init();

    while(1)
    {
		if(!(PIND & nCS))		/* Host bus cycle in progress */
		{
			if(!(PIND & nID))		/* nID asserted: place device ID on data bus and complete cycle */
			{
				DO_READ_CYCLE(PERIPHERAL_ID);
			}
			else					/* nID negated: register read/write cycle */
			{
				ku8 addr = GET_ADDRESS();

				if(PINC & nW)			/* nW negated - this is a read cycle */
				{
					cli();
					switch(addr)
					{
						case REG_DATA_A:
							fifo_read(&ctx_a);
							break;
							
						case REG_DATA_B:
							fifo_read(&ctx_b);
							break;
							
						default:
							DO_READ_CYCLE(host_regs[addr]);
							break;
					}
					sei();
				}
				else					/* nW asserted - this is a write cycle */
				{
					ku8 data = DATA_BUS_PIN;
					TERMINATE_BUS_CYCLE();
					
					cli();
					host_regs[addr] = data;
					switch(addr)
					{
						case REG_DATA_A:
							try_start_tx(&ctx_a, data);
							break;

						case REG_DATA_B:
							try_start_tx(&ctx_b, data);
							break;

						case REG_STATUS_A:
						case REG_STATUS_B:
							if(!(host_regs[REG_STATUS_A] & host_regs[REG_INT_CFG_A]) && 
							   !(host_regs[REG_STATUS_B] & host_regs[REG_INT_CFG_B]))
								HOST_IRQ_RELEASE();
							break;

						case REG_CFG:
							if(data & CFG_PWR_A)
								SET_LOW(PWR_PORT, nPWR_A);
							else
								SET_HIGH(PWR_PORT, nPWR_A);

							if(data & CFG_PWR_B)
								SET_LOW(PWR_PORT, nPWR_B);
							else
								SET_HIGH(PWR_PORT, nPWR_B);
							break;
					}
					sei();
				}
			}
		}
    }
}
