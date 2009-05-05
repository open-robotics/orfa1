/*!
 * @file usart.h
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <joerg@FreeBSD.ORG> wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.		Joerg Wunsch
 * ----------------------------------------------------------------------------
 *
 * \author Joerg Wunsch
 * \author Vladimir Ermakov <vooon341@gmail.com>
 *
 * \code #include "usart.h" \endcode
 */

#ifndef USART_H
#define USART_H
#ifdef AVR_IO

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <avr/io.h>

/*!
 * Use the following macros to determine the 'baud' parameter values
 * for uart_init()
 * @warning 'baud' SHOULD ALWAYS BE A CONSTANT or a lot of code
 * will be generated.
 */
#define USART_BAUD(baud) ((uint16_t)((F_CPU / (16.0 * (baud))) + 0.5) - 1)

/*!
 * Some typical baud rates
 * Confirm the cpu clock rate will support the desired baud rate
 */
#define B115200 (USART_BAUD(115200L))
#define B76800  (USART_BAUD(76800UL))
#define B57600  (USART_BAUD(57600UL))
#define B38400  (USART_BAUD(38400UL))
#define B28800  (USART_BAUD(28800U))
#define B19200  (USART_BAUD(19200U))
#define B14400  (USART_BAUD(14400))
#define B9600   (USART_BAUD(9600))
#define B4800   (USART_BAUD(4800))
#define B2400   (USART_BAUD(2400))

/*!
 * Double Speed operation
 * use the following macros to determine the 'baud' parameter values
 * for uartInit() for Double Speed operation.  The macros above will
 * also work just fine.
 * \warning 'baud' SHOULD ALWAYS BE A CONSTANT or a lot of code
 * will be generated.
 */
#define DOUBLE_SPEED_BIT (1<<15)
#define USART_2x_BAUD(baud) \
   (((uint16_t)((F_CPU / (8.0 * (baud))) + 0.5) - 1) | DOUBLE_SPEED_BIT)

/*!
 * Some typical baud rates
 * Confirm the cpu clock rate will support the desired baud rate
 */
#define B2x115200 (USART_2x_BAUD(115200L))
#define B2x76800  (USART_2x_BAUD(76800UL))
#define B2x57600  (USART_2x_BAUD(57600UL))
#define B2x38400  (USART_2x_BAUD(38400UL))
#define B2x28800  (USART_2x_BAUD(28800U))
#define B2x19200  (USART_2x_BAUD(19200U))
#define B2x14400  (USART_2x_BAUD(14400))
#define B2x9600   (USART_2x_BAUD(9600))
#define B2x4800   (USART_2x_BAUD(4800))
#define B2x2400   (USART_2x_BAUD(2400))

// USART file device
extern FILE usart_fdev;

/*!
 * This function initializes the USART
 *
 * \param baud baudrate divisor
 * \return void
 *
 * Example
 * \code
 * usart_init(B9600);
 * // or
 * usart_init(USART_BAUD(9600));
 * \endcode
 */
void usart_init(uint16_t baud);

/*!
 * Send one character to the UART.
 */
int usart_putchar(char c, FILE *stream);
int usart_putchar0(char c, FILE *stream);

/*!
 * Receive one character from the UART.
 */
int usart_getchar(FILE *stream);
int usart_getchar0(FILE *stream);

#endif // AVR_IO
#endif // USART_H

