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

#ifndef OR_AVR_M32_D
#error servoGPIO: unsupported platform
#endif

#include <avr/io.h>
#include <avr/interrupt.h>
#include "core/common.h"
#include "core/driver.h"
#include <stdint.h>
#include <stdbool.h>

#ifndef NDEBUG
#include "serialgate/common.h"
#endif

#define RESOLUTION_IN_TICKS 160
#define RESOLUTION_TIME     (F_CPU / RESOLUTION_IN_TICKS)
#define REST_IN_TICKS       (255-RESOLUTION_IN_TICKS)
#define MAX_TMR             (F_CPU/50/RESOLUTION_IN_TICKS)
#define MAXSERVO            (F_CPU/ 400/RESOLUTION_IN_TICKS)
#define MINSERVO            (F_CPU/2000/RESOLUTION_IN_TICKS)
#define WORKSPACE           (MAXSERVO+1)

#define CODE_FOR_SERVO(servo_id, port, pin) \
	if (gpio_servo_enb[servo_id]) { \
		if (tmp == 0) { \
			port |= (1<<pin); \
		} \
		if (tmp == gpio_servo_pos[servo_id]) { \
			port &= ~(1<<pin); \
		} \
	}

#define CODE_FOR_ENABLE(servo_id, ddr, pin) \
	if (gpio_servo_enb[servo_id]) { \
		ddr |= (1<<pin); \
	} else { \
		ddr &= ~(1<<pin); \
	}


static uint8_t gpio_servo_pos[16];
static bool gpio_servo_enb[16];
static uint16_t tmr, tmp;

//static GATE_RESULT driver_read(uint8_t reg, uint8_t* data, uint8_t* data_len);
static GATE_RESULT driver_write(uint8_t reg, uint8_t* data, uint8_t data_len);

static GATE_DRIVER driver = {
	.uid = 0x0031,
	.major_version = 1,
	.minor_version = 0,
//	.read = driver_read,
	.write = driver_write,
	.num_registers = 2,
};

ISR(SIG_OVERFLOW0)
{
	TCNT0 = REST_IN_TICKS;

	if (tmr < WORKSPACE*8) {
		if (tmr < WORKSPACE*4) {
			if (tmr < WORKSPACE*2) {
				if (tmr < WORKSPACE) {
					tmp = tmr;
					CODE_FOR_SERVO(0, PORTA, 0);
					CODE_FOR_SERVO(1, PORTA, 1);
				}
				else
				{
				    tmp = tmr - WORKSPACE;
					CODE_FOR_SERVO(2, PORTA, 2);
					CODE_FOR_SERVO(3, PORTA, 3);
				}
			}
			else
			{
				if (tmr < WORKSPACE*3) {
					tmp = tmr - WORKSPACE*2;
					CODE_FOR_SERVO(4, PORTA, 4);
					CODE_FOR_SERVO(5, PORTA, 5);
				}
				else
				{
					tmp = tmr - WORKSPACE*3;
					CODE_FOR_SERVO(6, PORTA, 6);
					CODE_FOR_SERVO(7, PORTA, 7);
				}
		    }
		}
		else
		{
			if (tmr < WORKSPACE*6) {
				if (tmr < WORKSPACE*5) {
					tmp = tmr - WORKSPACE*4;
					CODE_FOR_SERVO(8, PORTC, 7);
					CODE_FOR_SERVO(9, PORTC, 6);
				}
				else
				{
					tmp = tmr - WORKSPACE*5;
					CODE_FOR_SERVO(10, PORTC, 5);
					CODE_FOR_SERVO(11, PORTC, 4);
				}
			}
			else
			{
				if (tmr < WORKSPACE*7) {
				    tmp = tmr - WORKSPACE*6;
					CODE_FOR_SERVO(12, PORTB, 3);
					CODE_FOR_SERVO(13, PORTB, 2);
				}
				else
				{
				    tmp = tmr - WORKSPACE*7;
					CODE_FOR_SERVO(14, PORTD, 5);
					CODE_FOR_SERVO(15, PORTD, 4);
				}
			}
		}
	}

	if (tmr++ >= MAX_TMR) {
		tmr = 0; 
	}
}


static inline void set_enable(uint8_t n, bool enable)
{
	gpio_servo_enb[n] = (enable > 0) ? true : false;
	
	CODE_FOR_ENABLE(0,  DDRA, 0);
	CODE_FOR_ENABLE(1,  DDRA, 1);
	CODE_FOR_ENABLE(2,  DDRA, 2);
	CODE_FOR_ENABLE(3,  DDRA, 3);
	CODE_FOR_ENABLE(4,  DDRA, 4);
	CODE_FOR_ENABLE(5,  DDRA, 5);
	CODE_FOR_ENABLE(6,  DDRA, 6);
	CODE_FOR_ENABLE(7,  DDRA, 7);
	CODE_FOR_ENABLE(8,  DDRC, 7);
	CODE_FOR_ENABLE(9,  DDRC, 6);
	CODE_FOR_ENABLE(10, DDRC, 5);
	CODE_FOR_ENABLE(11, DDRC, 4);
	CODE_FOR_ENABLE(12, DDRB, 3);
	CODE_FOR_ENABLE(13, DDRB, 2);
	CODE_FOR_ENABLE(14, DDRD, 5);
	CODE_FOR_ENABLE(15, DDRD, 4);
}

static inline void set_position(uint8_t n, uint32_t pos)
{
	if (n > 15) 
		return;

	pos = pos * RESOLUTION_TIME/1000000;
	
	if (pos < MINSERVO)
		pos = MINSERVO;
	else if (pos > MAXSERVO)
		pos = MAXSERVO;

	gpio_servo_pos[n] = pos;
}


/* comment out now
static GATE_RESULT driver_read(uint8_t reg, uint8_t* data, uint8_t* data_len)
{
	*data_len = 0;
	return GR_OK;
}
*/

static GATE_RESULT driver_write(uint8_t reg, uint8_t* data, uint8_t data_len)
{
#ifndef NDEBUG
	debug("# servo_gpio->write(0x%02X, buf, %i)\n", reg, data_len);
#endif
	
	if (reg > 1) {
		return GR_NO_ACCESS;
	}

	if (reg == 0) {
		if (data_len != 3) {
			return GR_INVALID_DATA;
		}

		uint8_t byte = 0;
		while (byte < 2) {
			uint8_t bit = 0;
			while (bit < 8) {
				set_enable( (byte << 3) + bit , *data & 0x01);
				*data = *data>>1;
				bit++;
			}

			data++;
			byte++;
		}

		return GR_OK;
	}

	if (data_len < 3 || data_len > 252) {
		return GR_INVALID_DATA;
	}

	while (data_len) {
		set_position(*data, (data[1]<<8)|data[2]);
		data += 3;
		data_len -= 3;

		if (data_len < 3) {
			return GR_INVALID_DATA;
		}
	}

	return GR_OK;
}


GATE_RESULT init_servo_driver(void)
{
	for (uint8_t i=0; i < 16; i++) {
		set_enable(i, false);
		set_position(i, 1000);
    };

	// Prepare TIMER0 interrupt
	TCCR0 = 0x01;
	TCNT0 = REST_IN_TICKS;
	tmr = 0;
	TIMSK |= 0x01;

	return gate_driver_register(&driver);
}

