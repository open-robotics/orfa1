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
/** SERIAL file device
 * @file avr/serial_lld.h
 *
 * @code
 * #include <stdio.h>
 * #include "hal/serial.h"
 * stdin = stdout = &serial_fdev;
 * @endcode
 *
 * @author Joerg Wunsch
 * @author Vladimir Ermakov <vooon341@gmail.com>
 */

#ifndef SERIAL_LLD_H
#define SERIAL_LLD_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <avr/io.h>

/**
 * Use the following macros to determine the 'baud' parameter values
 * for uart_init()
 * @warning 'baud' SHOULD ALWAYS BE A CONSTANT or a lot of code
 * will be generated.
 */
#define SERIAL_BAUD(baud) ((uint16_t)((F_CPU / (16.0 * (baud))) + 0.5) - 1)

/** Typical baud rates
 * @note Confirm the cpu clock rate will support the desired baud rate
 * @{
 */
#define B115200 (SERIAL_BAUD(115200L))
#define B76800  (SERIAL_BAUD(76800UL))
#define B57600  (SERIAL_BAUD(57600UL))
#define B38400  (SERIAL_BAUD(38400UL))
#define B28800  (SERIAL_BAUD(28800U))
#define B19200  (SERIAL_BAUD(19200U))
#define B14400  (SERIAL_BAUD(14400))
#define B9600   (SERIAL_BAUD(9600))
#define B4800   (SERIAL_BAUD(4800))
#define B2400   (SERIAL_BAUD(2400))
///@}

/// Use baud rate autodetection
#define B_AUTO  0

/**
 * Double Speed operation
 * use the following macros to determine the 'baud' parameter values
 * for uartInit() for Double Speed operation.  The macros above will
 * also work just fine.
 * @warning 'baud' SHOULD ALWAYS BE A CONSTANT or a lot of code
 * will be generated.
 */
#define DOUBLE_SPEED_BIT (1<<15)
#define SERIAL_2x_BAUD(baud) \
   (((uint16_t)((F_CPU / (8.0 * (baud))) + 0.5) - 1) | DOUBLE_SPEED_BIT)

/** Typical 2x baud rates
 * @note Confirm the cpu clock rate will support the desired baud rate
 * @{
 */
#define B2x115200 (SERIAL_2x_BAUD(115200L))
#define B2x76800  (SERIAL_2x_BAUD(76800UL))
#define B2x57600  (SERIAL_2x_BAUD(57600UL))
#define B2x38400  (SERIAL_2x_BAUD(38400UL))
#define B2x28800  (SERIAL_2x_BAUD(28800U))
#define B2x19200  (SERIAL_2x_BAUD(19200U))
#define B2x14400  (SERIAL_2x_BAUD(14400))
#define B2x9600   (SERIAL_2x_BAUD(9600))
#define B2x4800   (SERIAL_2x_BAUD(4800))
#define B2x2400   (SERIAL_2x_BAUD(2400))
///@}

// indicate fdev
#define HAL_HAVE_SERIAL_FILE_DEVICE

/// Serial file device
extern FILE serial_lld_fdev;

/** This function initializes the serial device
 *
 * @param baud baudrate divisor
 * @return void
 *
 * Example
 * @code
 * serial_lld_init(B9600);
 * // or
 * serial_lld_init(SERIAL_BAUD(9600));
 * @endcode
 */
void serial_lld_init(uint16_t baud);

/** Send one character
 */
#define serial_lld_putchar(c) \
	serial_lld_fputchar(c, (FILE*)0)

/** Send one character to the UART.
 * @note for file device
 */
int serial_lld_fputchar(char c, FILE *stream);

/** Receive one character
 */
#define serial_lld_getchar() \
	serial_lld_fgetchar((FILE*)0)

/** Receive one character from the UART.
 * @note for file device
 */
int serial_lld_fgetchar(FILE *stream);

/** Check rxc buffer
 */
bool serial_lld_isempty(void);

#endif // SERIAL_LLD_H

