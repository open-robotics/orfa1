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

#include "registers/common.h"
#include "registers/driver.h"
#include <avr/io.h>
#include <stdint.h>
#include <stdbool.h>

static GATE_RESULT motor_driver_read(uint8_t reg, uint8_t* data, uint8_t* data_len);
static GATE_RESULT motor_driver_write(uint8_t reg, uint8_t* data, uint8_t data_len);

#define PWM1_REG   0x60
#define PWM2_REG   0x61
#define DIR1_REG   0x62
#define DIR2_REG   0x63

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

// timer1 OC1A config
#define COM_PWM1           ((1<<COM1A1)|(0<<COM1A0))
#define ATTACH_PWM1_PIN()  TCCR1A |= COM_PWM1
#define DETACH_PWM1_PIN()  TCCR1A &= ~COM_PWM1

// timer1 OC1A config
#define COM_PWM2           ((1<<COM1B1)|(0<<COM1B0))
#define ATTACH_PWM2_PIN()  TCCR1A |= COM_PWM2
#define DETACH_PWM2_PIN()  TCCR1A &= ~COM_PWM2

static uint8_t registers[] = {
	PWM1_REG,
	PWM2_REG,
	DIR1_REG,
	DIR2_REG
};

static GATE_DRIVER driver = {
	.uid = 0x0100, // motor id
	.major_version = 1,
	.minor_version = 0,
	.read = motor_driver_read,
	.write = motor_driver_write,
	.registers = registers,
	.num_registers = NUM_ELEMENTS(registers),
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
			*data = (uint8_t) PWM1_OC;
			break;

		case PWM2_REG:
			*data = (uint8_t) PWM1_OC;
			break;

		case DIR1_REG:
			*data = (DIR_PORT & DIR1_MASK) ? true : false;
			break;

		case DIR2_REG:
			*data = (DIR_PORT & DIR2_MASK) ? true : false;
			break;
	}
	
	return GR_OK;
}

static GATE_RESULT motor_driver_write(uint8_t reg, uint8_t* data, uint8_t data_len)
{
	// TODO: use ATOMIC_BLOCK()
	switch (reg)
	{
		case PWM1_REG:
			if (*data)
				ATTACH_PWM1_PIN();
			else
			{
				DETACH_PWM1_PIN();
				PWM_PORT &= ~PWM1_MASK;
			}
			PWM1_OC = (uint16_t) *data;
			break;

		case PWM2_REG:
			if (*data)
				ATTACH_PWM2_PIN();
			else
			{
				DETACH_PWM2_PIN();
				PWM_PORT &= ~PWM2_MASK;
			}
			PWM2_OC = (uint16_t) *data;
			break;

		case DIR1_REG:
			if (*data)
				DIR_PORT |= DIR1_MASK;
			else
				DIR_PORT &= ~DIR1_MASK;
			break;

		case DIR2_REG:
			if (*data)
				DIR_PORT |= DIR2_MASK;
			else
				DIR_PORT &= ~DIR2_MASK;
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

