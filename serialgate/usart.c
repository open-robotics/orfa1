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

// usart file device
FILE usart_fdev = FDEV_SETUP_STREAM(usart_putchar, usart_getchar, _FDEV_SETUP_RW);

// This function initializes the USART
void usart_init(uint16_t baud)
	{
	// disable the USART
	UCSRB = 0x00;
	UCSRA = 0x00;

	if ( baud & DOUBLE_SPEED_BIT )
		UCSRA |= (1 << U2X);

	// load the baudrate divisor register
	UBRRL = baud;

	// output the upper four bits of the baudrate divisor
	UBRRH = (baud >> 8) & 0x0F;

	// enable the USART0 transmitter & receiver
	UCSRB = (1 << TXEN) | (1 << RXEN);
	}

/*
 * Send character c down the UART Tx, wait until tx holding register
 * is empty.
 */
int usart_putchar(char c, FILE *stream)
{
	if ( c == '\n' )
		usart_putchar('\r', stream);
	loop_until_bit_is_set(UCSRA, UDRE);
	UDR = c;

	return 0;
}

int usart_putchar0(char c, FILE *stream)
{
	loop_until_bit_is_set(UCSRA, UDRE);
	UDR = c;
	return 0;
}

int usart_getchar(FILE *stream)
{
	uint8_t c;
	loop_until_bit_is_set(UCSRA, RXC);
	c = UDR;
	if ( c == '\r' )
		c = '\n';
	return c;
}

int usart_getchar0(FILE *stream)
{
	uint8_t c;
	loop_until_bit_is_set(UCSRA, RXC);
	c = UDR;
	return c;
}
#endif // AVR_IO

