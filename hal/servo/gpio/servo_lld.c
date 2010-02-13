/*
 *  ORFA -- Open Robotics Firmware Architecture
 *
 *  Copyright (c) 2009 Vladimir Ermakov, Andrey Demenev, Anton Botov
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
// vim: set noet:
/** Servo GPIO low level driver
 * @file servo/gpio/servo_lld.c
 *
 * @author Anton Botov <airsoft_ekb@mail.ru>
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>
#include <util/delay.h>

#include "servo_lld.h"

#define delay_us(x) _delay_us(x)

#define RESOLUTION_IN_TICKS 32
#define RESOLUTION_TIME     (F_CPU / RESOLUTION_IN_TICKS)
#define MAXSERVO            (F_CPU/ 400/RESOLUTION_IN_TICKS)
#define MINSERVO            (F_CPU/2000/RESOLUTION_IN_TICKS)
#define WORKSPACE           (MAXSERVO+1)

// -- driver module data --

static uint16_t gpio_servo_pos[16];
static bool gpio_servo_enb[16] = {
	false, false, false, false,
	false, false, false, false,
	false, false, false, false,
	false, false, false, false
};
static bool gpio_servo_enb_prev[16] = {
	false, false, false, false,
	false, false, false, false,
	false, false, false, false,
	false, false, false, false
};


static uint8_t pause0[40];
static uint8_t pause1[40];
static uint8_t mask0[40];
static uint8_t mask1[40];
static uint8_t iterator;

static uint8_t port_mask[16] = {
	_BV(0), _BV(1),
	_BV(2), _BV(3),
	_BV(4), _BV(5),
	_BV(6), _BV(7),
	_BV(7), _BV(6),
	_BV(5), _BV(4),
	_BV(3), _BV(2),
	_BV(5), _BV(4)
};

// -- handlers --

#define portHandlerOR(name, port, nextHandler, id) \
	void name(void) \
	{ \
		port |= mask0[id]; \
		OCR2 += pause0[id]; \
		handler = nextHandler; \
	}

#define portHandlerAND(name, port, nextHandler, id) \
	void name(void) \
	{ \
		port &= mask0[id]; \
		OCR2 += pause0[id]; \
		for (iterator=0; iterator<pause1[id]; iterator++) { \
			delay_us(4); \
		} \
		port &= mask1[id]; \
		handler = nextHandler; \
	}

#define portHandlers(process, port, nextProcess, id) \
	portHandlerAND(process##4, port, nextProcess##0, (id+4)) \
	portHandlerAND(process##3, port, process##4,     (id+3)) \
	portHandlerAND(process##2, port, process##3,     (id+2)) \
	portHandlerAND(process##1, port, process##2,     (id+1)) \
	portHandlerOR (process##0, port, process##1,     (id))

void processD0(void);
void (*handler)(void) = processD0;

ISR(SIG_OUTPUT_COMPARE2)
{
	 handler();
}

portHandlers(processAa, PORTA, processD,  0);
portHandlers(processAb, PORTA, processAa, 5);
portHandlers(processAc, PORTA, processAb, 10);
portHandlers(processAd, PORTA, processAc, 15);
portHandlers(processCa, PORTC, processAd, 20);
portHandlers(processCb, PORTC, processCa, 25);
portHandlers(processB,  PORTB, processCb, 30);
portHandlers(processD,  PORTD, processB,  35);

// -- [re]generate parameters --

static void generateParams(const uint8_t port_id, const uint8_t param_id)
{
	uint8_t maskX0=0,
			maskX1=0;
	uint16_t pauseX0=200,
			 pauseX1=200;
	
	if (gpio_servo_enb[port_id]) {
		maskX0 = port_mask[port_id]; 
		pauseX0 = gpio_servo_pos[port_id];
	}

	if (gpio_servo_enb[port_id+1]) {
		maskX1 = port_mask[port_id+1];
		pauseX1 = gpio_servo_pos[port_id+1];
	}

	if (pauseX0 > pauseX1) {
		uint16_t tmp16=pauseX0; 
		pauseX0 = pauseX1; 
		pauseX1 = tmp16;

		uint8_t tmp8=maskX0;
		maskX0 = maskX1;
		maskX1 = tmp8;
	}

	if (pauseX1-pauseX0 < 2) {
		if (pauseX0 > 400) {
			pause0[param_id+0] = 190;         mask0[param_id+0] = maskX0 | maskX1;
			pause0[param_id+1] = 190;         mask0[param_id+1] = 0xFF; pause1[param_id+1] = 0; mask1[param_id+1] = 0xFF;
			pause0[param_id+2] = pauseX0-380; mask0[param_id+2] = 0xFF; pause1[param_id+2] = 0; mask1[param_id+2] = 0xFF;

			if (pauseX0 == pauseX1) {
				pause0[param_id+3] = 590-pauseX0; mask0[param_id+3] = ~(maskX0|maskX1); pause1[param_id+3] = 0; mask1[param_id+3] = 0xFF;
			} else {
				pause0[param_id+3] = 590-pauseX0; mask0[param_id+3] = ~maskX0; pause1[param_id+3] = pauseX1-pauseX0; mask1[param_id+3] = ~maskX1;
			}

			pause0[param_id+4] = 10; mask0[param_id+4] = 0xFF; pause1[param_id+4] = 0; mask1[param_id+4] = 0xFF;
			
		} else if (pauseX0 > 200) {
			pause0[param_id+0] = 190;         mask0[param_id+0] = maskX0 | maskX1;
			pause0[param_id+1] = pauseX0-190; mask0[param_id+1] = 0xFF; pause1[param_id+1] = 0; mask1[param_id+1] = 0xFF;
			
			if (pauseX0 == pauseX1) {
				pause0[param_id+2] = 90; mask0[param_id+2] = ~(maskX0|maskX1); pause1[param_id+2] = 0; mask1[param_id+2] = 0xFF;
			} else {
				pause0[param_id+2] = 90; mask0[param_id+2] = ~maskX0; pause1[param_id+2] = pauseX1-pauseX0; mask1[param_id+2] = ~maskX1;
			}
			
			pause0[param_id+3] = 420-pauseX0; mask0[param_id+3] = 0xFF; pause1[param_id+3] = 0; mask1[param_id+3] = 0xFF;
			pause0[param_id+4] = 90;          mask0[param_id+4] = 0xFF; pause1[param_id+4] = 0; mask1[param_id+4] = 0xFF;

		} else {
			pause0[param_id+0] = pauseX0; mask0[param_id+0] = maskX0 | maskX1;
			
			if (pauseX0 == pauseX1) {
				pause0[param_id+1] = 250-pauseX0; mask0[param_id+1] = ~(maskX0|maskX1); pause1[param_id+1] = 0; mask1[param_id+1] = 0xFF;
			} else {
				pause0[param_id+1] = 250-pauseX0; mask0[param_id+1] = ~maskX0; pause1[param_id+1] = pauseX1-pauseX0; mask1[param_id+1] = ~maskX1;
			}
			
			pause0[param_id+2] = 100; mask0[param_id+2] = 0xFF; pause1[param_id+2] = 0; mask1[param_id+2] = 0xFF;
			pause0[param_id+3] = 150; mask0[param_id+3] = 0xFF; pause1[param_id+3] = 0; mask1[param_id+3] = 0xFF;
			pause0[param_id+4] = 100; mask0[param_id+4] = 0xFF; pause1[param_id+4] = 0; mask1[param_id+4] = 0xFF;
		}

	} else {
		if (pauseX1-pauseX0 > 235) {
			pause0[param_id+0] = 110;                 mask0[param_id+0] = maskX0 | maskX1;
			pause0[param_id+1] = pauseX0-110;         mask0[param_id+1] = 0xFF;    pause1[param_id+1] = 0; mask1[param_id+1] = 0xFF;
			pause0[param_id+2] = 230;                 mask0[param_id+2] = ~maskX0; pause1[param_id+2] = 0; mask1[param_id+2] = 0xFF;
			pause0[param_id+3] = pauseX1-pauseX0-230; mask0[param_id+3] = 0xFF;    pause1[param_id+3] = 0; mask1[param_id+3] = 0xFF;
			pause0[param_id+4] = 600-pauseX1;         mask0[param_id+4] = ~maskX1; pause1[param_id+4] = 0; mask1[param_id+4] = 0xFF;

		} else {
			if (pauseX0 > 440) {
				pause0[param_id+0] = 210;             mask0[param_id+0] = maskX0 | maskX1;
				pause0[param_id+1] = 210;             mask0[param_id+1] = 0xFF;    pause1[param_id+1] = 0; mask1[param_id+1] = 0xFF;
				pause0[param_id+2] = pauseX0-420;     mask0[param_id+2] = 0xFF;    pause1[param_id+2] = 0; mask1[param_id+2] = 0xFF;
				pause0[param_id+3] = pauseX1-pauseX0; mask0[param_id+3] = ~maskX0; pause1[param_id+3] = 0; mask1[param_id+3] = 0xFF;
				pause0[param_id+4] = 600-pauseX1;     mask0[param_id+4] = ~maskX1; pause1[param_id+4] = 0; mask1[param_id+4] = 0xFF;

			} else if (pauseX0 > 220) {
				uint16_t paus=600-pauseX1;
				pause0[param_id+0] = 210;             mask0[param_id+0] = maskX0 | maskX1;
				pause0[param_id+1] = pauseX0-210;     mask0[param_id+1] = 0xFF;    pause1[param_id+1] = 0; mask1[param_id+1] = 0xFF;
				pause0[param_id+2] = pauseX1-pauseX0; mask0[param_id+2] = ~maskX0; pause1[param_id+2] = 0; mask1[param_id+2] = 0xFF;		
				pause0[param_id+3] = paus >> 1;       mask0[param_id+3] = ~maskX1; pause1[param_id+3] = 0; mask1[param_id+3] = 0xFF;
				pause0[param_id+4] = paus-(paus>>1);  mask0[param_id+4] = 0xFF;    pause1[param_id+4] = 0; mask1[param_id+4] = 0xFF;

			} else {
				uint16_t paus=600-pauseX1;
				pause0[param_id+0] = 10;              mask0[param_id+0] = maskX0 | maskX1;
				pause0[param_id+1] = pauseX0-10;      mask0[param_id+1] = 0xFF;    pause1[param_id+1] = 0; mask1[param_id+1] = 0xFF;
				pause0[param_id+2] = pauseX1-pauseX0; mask0[param_id+2] = ~maskX0; pause1[param_id+2] = 0; mask1[param_id+2] = 0xFF;
				pause0[param_id+3] = paus >> 1;       mask0[param_id+3] = ~maskX1; pause1[param_id+3] = 0; mask1[param_id+3] = 0xFF;
				pause0[param_id+4] = paus-(paus>>1);  mask0[param_id+4] = 0xFF;    pause1[param_id+4] = 0; mask1[param_id+4] = 0xFF;
			}
		}
	}
}

static inline void generateParametersFor(uint8_t n)
{
	switch (n) {
		case 0:
		case 1:
			generateParams(0,  0);
			break;

		case 2:
		case 3:
			generateParams(2,  5);
			break;

		case 4:
		case 5:
			generateParams(4,  10);
			break;

		case 6:
		case 7:
			generateParams(6,  15);
			break;

		case 8:
		case 9:
			generateParams(8,  20);
			break;

		case 10:
		case 11:
			generateParams(10, 25);
			break;

		case 12:
		case 13:
			generateParams(12, 30);
			break;

		case 14:
		case 15:
			generateParams(14, 35);
			break;

		default:
			break;
	}
}

// -- servo controls --

#define enablePin(servo_id, ddr, pin) \
	if (gpio_servo_enb[servo_id]) { \
		ddr |= (1<<pin); \
	} else if (gpio_servo_enb_prev[servo_id]) { \
		ddr &= ~(1<<pin); \
	}

static void servo_set_enable(uint8_t n, bool enable)
{	
	gpio_servo_enb[n] = enable;

	enablePin(0,  DDRA, 0);
	enablePin(1,  DDRA, 1);
	enablePin(2,  DDRA, 2);
	enablePin(3,  DDRA, 3);
	enablePin(4,  DDRA, 4);
	enablePin(5,  DDRA, 5);
	enablePin(6,  DDRA, 6);
	enablePin(7,  DDRA, 7);
	enablePin(8,  DDRC, 7);
	enablePin(9,  DDRC, 6);
	enablePin(10, DDRC, 5);
	enablePin(11, DDRC, 4);
	enablePin(12, DDRB, 3);
	enablePin(13, DDRB, 2);
	enablePin(14, DDRD, 5);
	enablePin(15, DDRD, 4);

	// copy previous enable state
	memcpy(gpio_servo_enb_prev, gpio_servo_enb, 16);
}

// -- api --

void servo_lld_set_position(uint8_t n, uint32_t pos)
{
	if (n > SERVO_CHMAX)
		return;

	if (pos == 0) {
		servo_set_enable(n, false);
	} else if (!gpio_servo_enb[n]) {
		servo_set_enable(n, true);
	}

	pos = pos * RESOLUTION_TIME/1000000;
	
	if (pos < MINSERVO)
		pos = MINSERVO;
	else if (pos > MAXSERVO)
		pos = MAXSERVO;

	gpio_servo_pos[n] = pos;
	generateParametersFor(n);
}

void servo_lld_init(void)
{
	generateParams(0,  0);
	generateParams(2,  5);
	generateParams(4,  10);
	generateParams(6,  15);
	generateParams(8,  20);
	generateParams(10, 25);
	generateParams(12, 30);
	generateParams(14, 35);

	// Prepare TIMER2
	// 1/32 F clk, Normal mode
	// enable Timer2 compare isr
	TCCR2 = (0<<CS22)|(1<<CS21)|(1<<CS20);
	TCNT2 = 0;
	TIMSK |= (1<<OCIE2);
}

