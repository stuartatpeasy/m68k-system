/*
	PS2Controller.c - dual-channel PS/2 controller implementation
	
	This module contains firmware for an m68k-bus peripheral controller.
	
	
	(c) Stuart Wallace <stuartw@atom.net>, November 2015.
 */

//#define WITH_DEBUGGING				/* Enable debugging */

#include "PS2Controller.h"

volatile ctx_t ctx_a, ctx_b;
vu8 registers[8];


/*
	init() - initialise the chip following a reset
*/
void init(void)
{
	u8 i;
	
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

	/* Initialise register values */	
	for(i = 0; i < sizeof(registers) / sizeof(registers[0]); ++i)
		registers[i] = 0;
		
	/* Initialise PS/2 channel context */
	ctx_a.state				= state_idle;
	ctx_a.port				= &CHAN_A_PORT;
	ctx_a.pin				= &CHAN_A_PIN;
	ctx_a.ddr				= &CHAN_A_DDR;
	ctx_a.data_pin			= CHAN_A_DATA;
	ctx_a.data_regnum		= REG_DATA_A;
	ctx_a.command			= 0;
	ctx_a.command_pending	= 0;
	ctx_a.start_tx_fn		= start_tx_chan_a;
	ctx_a.reg_status		= &registers[REG_STATUS_A];
	ctx_a.reg_int_cfg		= &registers[REG_INT_CFG_A];

	ctx_a.irq				= irq_clk_a;
	ctx_a.fifo.rd			= 0;
	ctx_a.fifo.wr			= 0;
	
	ctx_b.state				= state_idle;
	ctx_b.port				= &CHAN_B_PORT;
	ctx_b.pin				= &CHAN_B_PIN;
	ctx_b.ddr				= &CHAN_B_DDR;
	ctx_b.data_pin			= CHAN_B_DATA;
	ctx_b.data_regnum		= REG_DATA_B;
	ctx_b.command			= 0;
	ctx_b.command_pending	= 0;
	ctx_b.start_tx_fn		= start_tx_chan_b;
	ctx_b.reg_status		= &registers[REG_STATUS_B];
	ctx_b.reg_int_cfg		= &registers[REG_INT_CFG_B];
	
	ctx_b.irq				= irq_clk_b;
	ctx_b.fifo.rd			= 0;
	ctx_b.fifo.wr			= 0;

	/* Enable interrupts on falling edge of PS/2 clock inputs */
	set_irq_edge(irq_clk_a, irq_edge_falling);
	set_irq_edge(irq_clk_b, irq_edge_falling);
	
	CHAN_A_IRQ_ENABLE();
	CHAN_B_IRQ_ENABLE();
	
	debug_init();
	
	sei();
}


/*
	set_irq_edge() - configure interrupts to trigger on the rising (IRQ_EDGE_RISING) or falling
	(IRQ_EDGE_FALLING) edge of INT1 and INT0 inputs.
*/
void set_irq_edge(const irq_t irq, const irq_edge_t edge)
{
	if(irq == irq_clk_a)
	{
		if(edge == irq_edge_falling)
			INT0_SET_FALLING_EDGE();
		else if(edge == irq_edge_rising)
			INT0_SET_RISING_EDGE();
	}
	else if(irq == irq_clk_b)
	{
		if(edge == irq_edge_falling)
			INT1_SET_FALLING_EDGE();
		else if(edge == irq_edge_rising)
			INT1_SET_RISING_EDGE();
	}
}


/*
	process_data() - process a byte of data received from a PS/2 channel.
*/
void process_data(volatile ctx_t *ctx)
{
	if(ctx->parity_received == ctx->parity_calc)
	{
		ku8 data = ctx->data,
			fifo_wrnext = ctx->fifo.wr + 1;

		/* Place received data in FIFO */
		if(fifo_wrnext != ctx->fifo.rd)
		{
			ctx->fifo.data[ctx->fifo.wr] = data;
			ctx->fifo.wr = fifo_wrnext;
			debug_puthexb(data);
			debug_putc('\n');

			*ctx->reg_status |= FLAG_RX;
			HOST_IRQ_ASSERT_IF(*ctx->reg_int_cfg & FLAG_RX);
		}
		else
		{
			/* FIFO is full; drop data */
			*ctx->reg_status |= FLAG_OVF;
			HOST_IRQ_ASSERT_IF(*ctx->reg_int_cfg & FLAG_OVF);
		}
	}
	else
	{
		/* Parity error */
		*ctx->reg_status |= FLAG_PAR_ERR;
		HOST_IRQ_ASSERT_IF(*ctx->reg_int_cfg & FLAG_PAR_ERR);

		debug_putc('!');
	}
}


/*
	process_clock_edge() - handle a clock transition on a PS/2 port.
	This function deals with the transmission and receipt of data on the ports.
*/
void process_clock_edge(volatile ctx_t *ctx)
{
	switch(++(ctx->state))
	{
		case state_idle:		/* Unreachable					*/
		case state_tx_busrq:	/* Unreachable (handled in ISR)	*/
			break;

		case state_rx_start:
			ctx->parity_calc = 1;		/* because odd parity */
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
			if(*ctx->pin & ctx->data_pin)
			{
				ctx->data |= 0x80;
				ctx->parity_calc ^= 1;
			}
			break;
		
		case state_rx_parity:
			ctx->parity_received = (*ctx->pin & ctx->data_pin) ? 1 : 0;
			break;
		
		case state_rx_stop:
			process_data(ctx);
			if(ctx->command_pending)
				ctx->start_tx_fn();
			else
				ctx->state = state_idle;
			break;
			
		case state_tx_start:
			SET_LOW(*ctx->port, ctx->data_pin);
			ctx->parity_calc = 1;		/* because odd parity */
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
				SET_HIGH(*ctx->port, ctx->data_pin);
				ctx->parity_calc ^= 1;
			}
			else
				SET_LOW(*ctx->port, ctx->data_pin);
			ctx->command >>= 1;
			break;
		
		case state_tx_parity:
			if(ctx->parity_calc)
				SET_HIGH(*ctx->port, ctx->data_pin);
			else
				SET_LOW(*ctx->port, ctx->data_pin);
			break;
		
		case state_tx_stop:
			SET_HIGH(*ctx->port, ctx->data_pin);
			SET_INPUT(*ctx->ddr, ctx->data_pin);
			break;
			
		case state_tx_ack:
			*ctx->reg_status |= FLAG_TX;
			ctx->state = state_idle;
			HOST_IRQ_ASSERT_IF(*ctx->reg_int_cfg & FLAG_TX);
				
			if(ctx->command_pending)
				ctx->start_tx_fn();
			break;
	}
}


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
	ISR for timer 0 overflow - handles channel A data-transmission bus requests.
*/
ISR(TIMER0_OVF_vect)
{
	/* Disable timer 0 */
	SET_LOW(TIMSK, _BV(TOIE0));
	SET_LOW(TCCR0, _BV(CS02) | _BV(CS01) | _BV(CS00));

	DATA_A_SET_LOW();			/* Pull the channel A  data line low	*/
	DATA_A_SET_OUTPUT();		/* Set channel A data line as output	*/
	CLK_A_SET_HIGH();			/* Pull the channel A clock line high	*/
	CLK_A_SET_INPUT();			/* Set channel A clock as input			*/

	/* Re-enable channel A clock interrupt */
	CHAN_A_IRQ_ENABLE();
}


/*
	ISR for timer 2 overflow - handles channel B data-transmission bus requests.
*/
ISR(TIMER2_OVF_vect)
{
	SET_LOW(TIMSK, _BV(TOIE2));
	SET_LOW(TCCR2, _BV(CS02) | _BV(CS01) | _BV(CS00));	/* Check this */
	
	DATA_B_SET_LOW();		/* Pull the channel B data line low		*/
	DATA_B_SET_OUTPUT();	/* Set channel B data line as output	*/
	CLK_B_SET_HIGH();		/* Pull the channel B clock line high	*/
	CLK_B_SET_INPUT();		/* Set channel B clock as input			*/

	/* Re-enable channel B clock interrupt */
	CHAN_B_IRQ_ENABLE();
}


/*
	start_tx_chan_a() - start transmitting a byte on channel A.
*/
void start_tx_chan_a()
{
	CHAN_A_IRQ_DISABLE();
	ctx_a.command_pending = 0;
	
	/* Start channel A command transmission */
	CLK_A_SET_OUTPUT();				/* Set channel A clock line to output				*/
	CLK_A_SET_LOW();				/* Pull PS/2 channel A clock line low				*/

	/* Generate an interrupt after 150us */
	TCNT0 = 106;					/* 106 = 256 - 150, ie. overflow after 150 counts	*/
	SET_HIGH(TIMSK, _BV(TOIE0));	/* Enable interrupt on timer 0 overflow				*/
	TCCR0 = _BV(CS01);				/* Set timer clock source to f(clk)/8 (=1MHz)		*/

	ctx_a.state = state_tx_busrq;
}


/*
	start_tx_chan_b() - start transmitting a byte on channel B.
*/
void start_tx_chan_b()
{
	CHAN_B_IRQ_DISABLE();
	ctx_b.command_pending = 0;
	
	/* Start channel B command transmission */
	CLK_B_SET_OUTPUT();				/* Set channel B clock line to output				*/
	CLK_B_SET_LOW();				/* Pull PS/2 channel B clock line low				*/
	
	/* Generate an interrupt after 150us */
	TCNT2 = 106;					/* 106 = 256 - 150, ie. overflow after 150 counts	*/
	SET_HIGH(TIMSK, _BV(TOIE2));	/* Enable interrupt on timer 2 overflow				*/
	TCCR2 = _BV(CS01);				/* Set timer clock source to f(clk)/8 (=1MHz)		*/
	
	ctx_b.state = state_tx_busrq;
}


/*
	main() - main loop: manage communications with the host
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
					switch(addr)
					{
						case REG_DATA_A:
							cli();
							if(ctx_a.fifo.rd == ctx_a.fifo.wr)
							{
								DO_READ_CYCLE(0);		/* FIFO is empty */
								registers[REG_STATUS_A] &= ~FLAG_RX;
							}
							else
							{
								DO_READ_CYCLE(ctx_a.fifo.data[ctx_a.fifo.rd++]);
								if(ctx_a.fifo.rd == ctx_a.fifo.wr)
									registers[REG_STATUS_A] &= ~FLAG_RX;
							}
							sei();
							break;
							
						case REG_DATA_B:
							cli();
							if(ctx_b.fifo.rd == ctx_b.fifo.wr)
							{
								DO_READ_CYCLE(0);		/* FIFO is empty */
								registers[REG_STATUS_B] &= ~FLAG_RX;
							}
							else
							{
								DO_READ_CYCLE(ctx_b.fifo.data[ctx_b.fifo.rd++]);
								if(ctx_b.fifo.rd == ctx_b.fifo.wr)
									registers[REG_STATUS_B] &= ~FLAG_RX;
							}
							sei();
							break;
							
						default:
							DO_READ_CYCLE(registers[addr]);
							break;
					}
				}
				else					/* nW asserted - this is a write cycle */
				{
					ku8 data = DATA_BUS_PIN;
					TERMINATE_BUS_CYCLE();
					
					cli();
					registers[addr] = data;
					switch(addr)
					{
						case REG_DATA_A:
							/* Try to start channel A command transmission */
							ctx_a.command = data;
							if(ctx_a.state == state_idle)
								start_tx_chan_a();
							else
								ctx_a.command_pending = 1;
							break;
							
						case REG_DATA_B:
							/* Try to start channel B command transmission */
							ctx_b.command = data;
							if(ctx_b.state == state_idle)
								start_tx_chan_b();
							else
								ctx_b.command_pending = 1;
							break;
						
						case REG_STATUS_A:
						case REG_STATUS_B:
							if(!(registers[REG_STATUS_A] & registers[REG_INT_CFG_A]) && 
							   !(registers[REG_STATUS_B] & registers[REG_INT_CFG_B]))
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
