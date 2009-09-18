/*
 *  ORFA -- Open Robotics Firmware Architecture
 *  Based on Joerg Wunsch's UART lib (THE BEER-WARE LICENSE)
 *
 *  Copyright (c) 2009 Vladimir Ermakov, Andrey Demenev
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 *  THE SOFTWARE.
 *****************************************************************************/

#ifdef AVR_IO

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <avr/io.h>

#include "usart.h"

#ifdef SG_ENABLE_IRQ
#include <avr/interrupt.h>
#include <util/atomic.h>
#include "cbuf.h"

static volatile cbf_t rx_cbf;

bool usart_isempty(void)
{
	ATOMIC_BLOCK(ATOMIC_FORCEON) {
		bool ret=cbf_isempty(&rx_cbf);
	}
	return ret;
}

ISR(USART_RXC_vect)
{
	uint8_t c=GATE_UDR;
	cbf_put(&rx_cbf, c);
}
#endif

// usart file device
FILE usart_fdev = FDEV_SETUP_STREAM(usart_putchar, usart_getchar, _FDEV_SETUP_RW);

// This function initializes the USART
void usart_init(uint16_t baud)
{
	// disable the USART
	GATE_UCSRB = 0x00;
	GATE_UCSRA = 0x00;

	if ( baud & DOUBLE_SPEED_BIT )
		GATE_UCSRA |= (1 << U2X);

	// load the baudrate divisor register
	GATE_UBRRL = baud;

	// output the upper four bits of the baudrate divisor
	GATE_UBRRH = (baud >> 8) & 0x0F;

#ifndef SG_ENABLE_IRQ
	// enable the USART0 transmitter & receiver
	GATE_UCSRB = (1 << TXEN) | (1 << RXEN);
#else
	// init rx buffer
	cbf_init(&rx_cbf);
	// enable the USART0 transmitter & receiver & receiver interrupt
	GATE_UCSRB = (1 << RXCIE) | (1 << TXEN) | (1 << RXEN);
#endif
}

/*
 * Send character c down the UART Tx, wait until tx holding register
 * is empty.
 */
int usart_putchar(char c, FILE *stream)
{
	(void)stream;
	if ( c == '\n' )
		usart_putchar('\r', stream);
	loop_until_bit_is_set(GATE_UCSRA, UDRE);
	GATE_UDR = c;

	return 0;
}

int usart_putchar0(char c, FILE *stream)
{
	(void)stream;
	loop_until_bit_is_set(GATE_UCSRA, UDRE);
	GATE_UDR = c;
	return 0;
}

int usart_getchar(FILE *stream)
{	
	uint8_t c;
	(void)stream;

#ifndef SG_ENABLE_IRQ
	loop_until_bit_is_set(GATE_UCSRA, RXC);
	c = GATE_UDR;
#else
	ATOMIC_BLOCK(ATOMIC_FORCEON) {
		c = cbf_get(&usart_cbf);
	}
#endif

	return c;
}

int usart_getchar0(FILE *stream)
{
	uint8_t c;
	(void)stream;
	loop_until_bit_is_set(GATE_UCSRA, RXC);
	c = GATE_UDR;
	return c;
}
#endif // AVR_IO

