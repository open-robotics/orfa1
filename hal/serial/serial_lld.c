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

#include <stdlib.h>
#include <avr/io.h>
#include <avr/pgmspace.h>

#include "serial_lld.h"
#include "serial_lld_config.h"

#include <avr/interrupt.h>
#include <util/atomic.h>
#include "lib/cbuf.h"


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

#ifndef HAL_SERIAL_NISR
// static volatile give compilation errors :(
static cbf_t rx_cbf;

bool serial_lld_isempty(void)
{
	bool ret;

	ATOMIC_BLOCK(ATOMIC_FORCEON) {
		ret = cbf_isempty(&rx_cbf);
	}
	return ret;
}

ISR(SERIAL_RXC_vect)
{
	uint8_t c=SERIAL_UDR;
	cbf_put(&rx_cbf, c);
}
#else
bool serial_lld_isempty(void)
{
	return bit_is_clear(SERIAL_UCSRA, RXC);
}
#endif

// autodetecting baud rate
// need send "\x0d\x0d\x0d\x0d\x0d\x0d\x0d\x0d"
static uint16_t detect_baud_rate(void)
{
	uint16_t min;
	int16_t baud;
	uint8_t count;

	SERIAL_DDR &= ~_BV(SERIAL_RXD_BIT);
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
        : [bit] "I" (SERIAL_RXD_BIT), 
          [port] "I" (_SFR_IO_ADDR(SERIAL_PIN))
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
	  [bit] "I" (SERIAL_RXD_BIT), [count] "r" (count),
      [port] "I" (_SFR_IO_ADDR(SERIAL_PIN))
	: "r24","r25","r20","r21","r19"
	);

	return min / 16 - 1;
}

// serial_lld file device
FILE serial_lld_fdev = FDEV_SETUP_STREAM(serial_lld_fputchar, serial_lld_fgetchar, _FDEV_SETUP_RW);

// This function initializes the SERIAL
void serial_lld_init(uint16_t baud)
{
	// disable the SERIAL
	SERIAL_UCSRB = 0x00;
	SERIAL_UCSRA = 0x00;
    if (!baud) {
        baud = detect_baud_rate();
    }

	if ( baud & DOUBLE_SPEED_BIT )
		SERIAL_UCSRA |= (1 << U2X);

	// load the baudrate divisor register
	SERIAL_UBRRL = baud;

	// output the upper four bits of the baudrate divisor
	SERIAL_UBRRH = (baud >> 8) & 0x0F;

#ifndef HAL_SERIAL_NISR
	// init rx buffer
	cbf_init(&rx_cbf);
	// enable the SERIAL0 transmitter & receiver & receiver interrupt
	SERIAL_UCSRB = (1 << RXCIE) | (1 << TXEN) | (1 << RXEN);
#else
	// enable the SERIAL0 transmitter & receiver
	SERIAL_UCSRB = (1 << TXEN) | (1 << RXEN);
#endif
}

/*
 * Send character c down the UART Tx, wait until tx holding register
 * is empty.
 */
int serial_lld_fputchar(char c, FILE *stream)
{
	(void)stream;
	if ( c == '\n' )
		serial_lld_fputchar('\r', stream);
	loop_until_bit_is_set(SERIAL_UCSRA, UDRE);
	SERIAL_UDR = c;

	return 0;
}

int serial_lld_fgetchar(FILE *stream)
{	
	uint8_t c;
	(void)stream;

#ifdef HAL_SERIAL_NISR
	loop_until_bit_is_set(SERIAL_UCSRA, RXC);
	c = SERIAL_UDR;
#else
	ATOMIC_BLOCK(ATOMIC_FORCEON) {
		c = cbf_get(&rx_cbf);
	}
#endif

	return c;
}

