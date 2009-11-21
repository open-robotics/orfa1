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
#include <stdlib.h>
#include <stdbool.h>
#include <avr/io.h>
#include <avr/pgmspace.h>

#include "usart.h"

#ifndef SG_DISABLE_IRQ
#include <avr/interrupt.h>
#include <util/atomic.h>
#include "cbuf.h"
#endif


PROGMEM int16_t baud_cycles[] = {
	(B2400 + 1) * 16,
	(B4800 + 1) * 16,
	(B9600 + 1) * 16,
	(B14400 + 1) * 16,
	(B19200 + 1) * 16,
	(B28800 + 1) * 16,
	(B38400 + 1) * 16,
	(B57600 + 1) * 16,
	(B76800 + 1) * 16,
	(B115200 + 1) * 16,
	0,
};

#ifndef SG_DISABLE_IRQ
// static volatile give compilation errors :(
static cbf_t rx_cbf;

bool usart_isempty(void)
{
	bool ret;

	ATOMIC_BLOCK(ATOMIC_FORCEON) {
		ret = cbf_isempty(&rx_cbf);
	}
	return ret;
}

ISR(GATE_RXC_vect)
{
	uint8_t c=GATE_UDR;
	cbf_put(&rx_cbf, c);
}
#endif

// autodetecting baud rate
// need send "\x0d\x0d\x0d\x0d\x0d\x0d\x0d\x0d"
uint16_t detect_baud_rate(void)
{
	uint16_t min;
	int16_t baud;
	uint8_t count;

	USART_DDR &= ~_BV(USART_RXD_BIT);
	asm volatile("\
		ldi %[count], 8				\n\
		in __tmp_reg__, 0x3F		\n\
		cli							\n\
		ldi %A[min], 0xFF			\n\
		ldi %B[min], 0xFF			\n\
2:		ldi %A[baud], 0				\n\
		ldi %B[baud], 0				\n\
1:		sbis %[port], %[bit]		\n\
		rjmp 1b						\n\
1:		sbic %[port], %[bit]		\n\
		rjmp 1b						\n\
1:		adiw %[baud], 1				\n\
		breq 2b						\n\
		sbis %[port], %[bit]		\n\
		rjmp 1b						\n\
		cp  %A[baud], %A[min]		\n\
		cpc %B[baud], %B[min]		\n\
		brcc 1f						\n\
		movw %[min], %[baud]		\n\
1:		dec %[count]				\n\
		brne 2b						\n\
		out 0x3F, __tmp_reg__		\n\
	"
        : [min] "=&w" (min), [baud] "=&w" (baud), [count] "=&r" (count)
        : [bit] "I" (USART_RXD_BIT), 
          [port] "I" (_SFR_IO_ADDR(USART_PIN))
    );

	baud = min * 6;
	min = 0xFFFF;
	int16_t *cur, cycles;
	for (cur = baud_cycles; ; cur++) {
		cycles = pgm_read_word(cur);
		if (!cycles) break;
		if (abs(min - baud) > abs(cycles - baud)) {
			min = cycles;
		}
	}

#define INACTIVITY_DELAY ((F_CPU / 125) / 13)

	asm volatile("\
2:		ldi	R24, %[delay0]		\n\
		ldi	R25, %[delay1]		\n\
		ldi	R20, %[delay2]		\n\
		ldi	R21, %[delay3]		\n\
1:		sbis %[port], %[bit]	\n\
		rjmp 2b					\n\
		sbiw R24, 1				\n\
		sbci R20, 0				\n\
		sbci R21, 0				\n\
		ldi R19, 0				\n\
		or R19, R24				\n\
		or R19, R25				\n\
		or R19, R20				\n\
		or R19, R21				\n\
		brne 1b					\n\
	"
	:
	: [delay0] "M" (INACTIVITY_DELAY & 0xFFU),
	  [delay1] "M" ((INACTIVITY_DELAY >> 8) & 0xFFU),
	  [delay2] "M" ((INACTIVITY_DELAY >> 16) & 0xFFU),
	  [delay3] "M" ((INACTIVITY_DELAY >> 24) & 0xFFU),
	  [bit] "I" (USART_RXD_BIT), [count] "r" (count),
      [port] "I" (_SFR_IO_ADDR(USART_PIN))
	: "r24","r25","r20","r21","r19"
	);

	return min / 16 - 1;
}

// usart file device
FILE usart_fdev = FDEV_SETUP_STREAM(usart_putchar, usart_getchar, _FDEV_SETUP_RW);

// This function initializes the USART
void usart_init(uint16_t baud)
{
	// disable the USART
	GATE_UCSRB = 0x00;
	GATE_UCSRA = 0x00;
    if (!baud) {
        baud = detect_baud_rate();
    }

	if ( baud & DOUBLE_SPEED_BIT )
		GATE_UCSRA |= (1 << U2X);

	// load the baudrate divisor register
	GATE_UBRRL = baud;

	// output the upper four bits of the baudrate divisor
	GATE_UBRRH = (baud >> 8) & 0x0F;

#ifndef SG_DISABLE_IRQ
	// init rx buffer
	cbf_init(&rx_cbf);
	// enable the USART0 transmitter & receiver & receiver interrupt
	GATE_UCSRB = (1 << RXCIE) | (1 << TXEN) | (1 << RXEN);
#else
	// enable the USART0 transmitter & receiver
	GATE_UCSRB = (1 << TXEN) | (1 << RXEN);
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

#ifdef SG_DISABLE_IRQ
	loop_until_bit_is_set(GATE_UCSRA, RXC);
	c = GATE_UDR;
#else
	ATOMIC_BLOCK(ATOMIC_FORCEON) {
		c = cbf_get(&rx_cbf);
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

