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
/** Platform-specific serial macroses
 * @file avr/serial_lld_config.h
 *
 * @author Andrey Demenev
 */

#ifndef SERIAL_LLD_CONFIG_H
#define SERIAL_LLD_CONFIG_H

#include <avr/io.h>

#ifdef OR_AVR_M32_D
	#define SERIAL_UDR       UDR
	#define SERIAL_UCSRA     UCSRA
	#define SERIAL_UCSRB     UCSRB
	#define SERIAL_UBRRL     UBRRL
	#define SERIAL_UBRRH     UBRRH
	#define SERIAL_RXC_vect  USART_RXC_vect

	#define SERIAL_DDR       DDRD
	#define SERIAL_PIN       PIND
	#define SERIAL_RXD_BIT   0
#endif

#if defined(OR_AVR_M128_S) || \
	defined(OR_AVR_M128_DS)
	#define SERIAL_UDR       UDR1
	#define SERIAL_UCSRA     UCSR1A
	#define SERIAL_UCSRB     UCSR1B
	#define SERIAL_UBRRL     UBRR1L
	#define SERIAL_UBRRH     UBRR1H
	#define SERIAL_RXC_vect  USART1_RX_vect

	#define SERIAL_DDR       PORTD
	#define SERIAL_PIN       PIND
	#define SERIAL_RXD_BIT   2
#endif

#endif // SERIAL_LLD_CONFIG_H

