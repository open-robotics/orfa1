/*!
 * @file usart.c
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <joerg@FreeBSD.ORG> wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.		Joerg Wunsch
 * ----------------------------------------------------------------------------
 *
 * @author Joerg Wunsch
 * @author Vladimir Ermakov <vooon341@gmail.com>
 *
 */

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

