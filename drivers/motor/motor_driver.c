/*
 *  ORFA -- Open Robotics Firmware Architecture
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
/** RoboMD2 motor driver
 * @file motor_driver.c
 * @author Vladimir Ermakov
 *
 * @todo use ATOMIC_BLOCK()
 *
 * @note New in version 1.1:
 *       now you can set all registers in one request
 */

/**
 * @ingroup Drivers
 * @defgroup Motor Motor driver
 *
 * UID: 0x0060
 *
 * @{
 */

#include "core/common.h"
#include "core/driver.h"
#include <avr/io.h>
#include <stdint.h>
#include <stdbool.h>

static GATE_RESULT
motor_driver_read(uint8_t reg, uint8_t* data, uint8_t* data_len);
static GATE_RESULT
motor_driver_write(uint8_t reg, uint8_t* data, uint8_t data_len);

/// PWM register channel 1
#define PWM1_REG   0x00
/// PWM register channel 2
#define PWM2_REG   0x01
/// Direction register channel 1
#define DIR1_REG   0x02
/// Direction register channel 2
#define DIR2_REG   0x03

#define DIR_PORT   PORTB
#define DIR_DDR    DDRB
#define DIR1_MASK  _BV(PB0)
#define DIR2_MASK  _BV(PB1)
#define DIR_MASK   (DIR1_MASK|DIR2_MASK)

#define PWM1_OC    OCR1A
#define PWM2_OC    OCR1B
#define PWM_PORT   PORTD
#define PWM_DDR    DDRD
#define PWM1_MASK  _BV(PD4) // OC1A
#define PWM2_MASK  _BV(PD5) // OC1B
#define PWM_MASK   (PWM1_MASK|PWM2_MASK) // avr/iom32.h OC1A, OC1B

// Timer1 OC1A, OC1B config
#define COM_PWM1           ((1<<COM1A1)|(0<<COM1A0))
#define COM_PWM2           ((1<<COM1B1)|(0<<COM1B0))
#define ATTACH_PWM_PIN(ch)  TCCR1A |= COM_PWM##ch
#define DETACH_PWM_PIN(ch)  TCCR1A &= ~COM_PWM##ch

#define setDirection(ch, value) \
	if (value) \
		DIR_PORT |= DIR##ch##_MASK; \
	else \
		DIR_PORT &= ~DIR##ch##_MASK

#define setPwm(ch, value) \
	if (value) \
		ATTACH_PWM_PIN(ch); \
	else { \
		DETACH_PWM_PIN(ch); \
		PWM_PORT &= ~PWM##ch##_MASK; \
	} \
	PWM##ch##_OC = (uint16_t) value

#define getDirection(ch) \
	((DIR_PORT & DIR##ch##_MASK) ? true : false)

#define getPwm(ch) \
	((uint8_t) PWM##ch##_OC)

static GATE_DRIVER driver = {
	.uid = 0x0060, // motor id
	.major_version = 1,
	.minor_version = 1,
	.read = motor_driver_read,
	.write = motor_driver_write,
	.num_registers = 4,
};

static GATE_RESULT motor_driver_read(uint8_t reg, uint8_t* data, uint8_t* data_len)
{
	if (!*data_len) {
		return GR_OK;
	}
	
	*data_len = 1;
	switch (reg)
	{
		case PWM1_REG:
			*data = getPwm(1);
			break;

		case PWM2_REG:
			*data = getPwm(2);
			break;

		case DIR1_REG:
			*data = getDirection(1);
			break;

		case DIR2_REG:
			*data = getDirection(2);
			break;
	}
	
	return GR_OK;
}

static GATE_RESULT motor_driver_write(uint8_t reg, uint8_t* data, uint8_t data_len)
{
	if (!data_len)
		return GR_INVALID_ARG;

	if (data_len == 4) {
		setPwm(1, data[0]);
		setPwm(2, data[1]);
		setDirection(1, data[2]);
		setDirection(2, data[3]);
	}

	switch (reg)
	{
		case PWM1_REG:
			setPwm(1, *data);
			break;

		case PWM2_REG:
			setPwm(2, *data);
			break;

		case DIR1_REG:
			setDirection(1, *data);
			break;

		case DIR2_REG:
			setDirection(2, *data);
			break;
	}
	
	return GR_OK;
}

GATE_RESULT init_motor_driver(void)
{
	// init i/o lines
	DIR_PORT &= ~DIR1_MASK;
	DIR_DDR |= DIR_MASK;
	PWM_PORT &= ~PWM_MASK;
	PWM_DDR |= PWM_MASK;

	// clear pwm registers
	PWM1_OC = 0;
	PWM2_OC = 0;

	// Timer/Counter-1 init
	//
	// COM1x1:COM1x0 = 1:0 => OC1x connected to output pin, reset on compare
	// FOC1A:FOC1B = 0:0 => Don't force output compare event
	// WGM13:WGM12:WGM11:WGM10 = 0:1:0:1 => Fast PWM 8-bit
	// ICNC1 = 0 => Input Capture Noise Canceller disabled (because not needed)
	// ICES1 = 0 => Input Capture Edge Select set to "falling" mode (no matter)
	// CS12:CS11:CS10 = 0:1:0 => Timer "on", 1/8 prescaling (1 clock every 8 cpu tact)	// /8
	TCNT1 = 0;
	TCCR1A = (0<<WGM11)|(1<<WGM10);
	TCCR1B = (0<<WGM13)|(1<<WGM12)|(0<<CS12)|(1<<CS11)|(0<<CS10);

	return gate_driver_register(&driver);
}

