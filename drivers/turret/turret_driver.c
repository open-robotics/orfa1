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
/** Turret driver for TermiTiger
 * @file turret_driver.c
 *
 * @author Vladimir Ermakov
 */

#include "core/common.h"
#include "core/driver.h"
#include "core/ports.h"
#include "core/scheduler.h"
#include <avr/io.h>
#include <stdint.h>
#include <stdbool.h>

/* NOTE:
 * PORTC configuration for TermiTiger (aka TT)
 * PC4 — canon button (normal grounded, need pull up)
 * PC5 — canon motor
 * PC6 — turret left
 * PC7 — turret right
 * 
 * PORTA:
 * PA1 — turret center sensor
 */
#define MOTOR_PORT  PORTC
#define MOTOR_DDR   DDRC
#define LEFT_MASK   _BV(PC6) // OUT
#define RIGHT_MASK  _BV(PC7) // OUT

#define SENSOR_PORT PORTA
#define SENSOR_DDR  DDRA
#define SENSOR_PIN  PINA
#define SENSOR_MASK _BV(PA1) // IN

// See port specs
#define RESERVE_MOTOR_PORT 3 // PORTC
#define RESERVE_MOTOR_MASK (LEFT_MASK | RIGHT_MASK)
#define RESERVE_SENSOR_PORT 0 // PORTA
#define RESERVE_SENSOR_MASK SENSOR_MASK

#define motor_left()  MOTOR_PORT = (MOTOR_PORT | LEFT_MASK) & ~RIGHT_MASK
#define motor_right() MOTOR_PORT = (MOTOR_PORT | RIGHT_MASK) & ~LEFT_MASK
#define motor_off()   MOTOR_PORT &= ~(LEFT_MASK | RIGHT_MASK)

#define get_sensor_state() (SENSOR_PIN & SENSOR_MASK)


static GATE_RESULT
turret_driver_read(uint8_t reg, uint8_t* data, uint8_t* data_len);
static GATE_RESULT
turret_driver_write(uint8_t reg, uint8_t* data, uint8_t data_len);
static void turret_task_func(void);

static GATE_DRIVER turret_driver = {
	.uid = 0xff02, // turret id
	.major_version = 1,
	.minor_version = 0,
	.read = turret_driver_read,
	.write = turret_driver_write,
	.num_registers = 1,
};

static GATE_TASK turret_task = {
	.task = turret_task_func,
};

static enum {
	T_STOP=0,    //!< stop turret
	T_LEFT,      //!< turn left
	T_RIGHT,     //!< turn right
	T_CENTER,    //!< go to center
} operation = T_STOP;

static bool go_to_center=false;

static GATE_RESULT
turret_driver_read(uint8_t reg, uint8_t* data, uint8_t* data_len)
{
	if (!*data_len) {
		return GR_OK;
	}
	
	*data_len = 1;
	*data = (uint8_t) operation;

	return GR_OK;
}

static GATE_RESULT
turret_driver_write(uint8_t reg, uint8_t* data, uint8_t data_len)
{	
	if (data_len != 1) {
		return GR_INVALID_ARG;
	}

	go_to_center = false;
	operation = *data;
	switch (operation) {
		case T_STOP:
			motor_off();
			break;

		case T_LEFT:
			motor_left();
			break;

		case T_RIGHT:
			motor_right();
			break;

		case T_CENTER:
			go_to_center = true;
			motor_off();
			break;

		default:
			operation = T_STOP;
			break;
	}

	return GR_OK;
}

static void turret_task_func(void)
{
	// TODO: go to the center proc
}

GATE_RESULT init_turret_driver(void)
{
	// Port & DDR init
	MOTOR_PORT &= ~(LEFT_MASK | RIGHT_MASK);
	MOTOR_DDR |= LEFT_MASK | RIGHT_MASK;

	SENSOR_PORT &= ~SENSOR_MASK;
	SENSOR_DDR &= ~SENSOR_MASK;

	// reserve pinouts
	gate_port_reserve(RESERVE_MOTOR_PORT, 
			RESERVE_MOTOR_MASK, RESERVE_MOTOR_MASK);
	gate_port_reserve(RESERVE_SENSOR_PORT, 
			RESERVE_SENSOR_MASK, RESERVE_SENSOR_MASK);

	gate_task_register(&turret_task);

	return gate_driver_register(&turret_driver);
}

