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
/** Canon driver for TermiTiger
 * @file canon_driver.c
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
 * PC7 — turret rigth
 */
#define CANON_PORT  PORTC
#define CANON_PIN   PINC
#define CANON_DDR   DDRC
#define MOTOR_MASK  _BV(PC5) // OUT
#define BUTTON_MASK _BV(PC4) // IN

// See port specs
#define RESERVE_PORT 3 // PORTC
#define RESERVE_MASK (MOTOR_MASK | BUTTON_MASK)

#define motor_on()  CANON_PORT |= MOTOR_MASK
#define motor_off() CANON_PORT &= ~MOTOR_MASK

#define get_button_state() (CANON_PIN & BUTTON_MASK)

static GATE_RESULT
canon_driver_read(uint8_t reg, uint8_t* data, uint8_t* data_len);
static GATE_RESULT
canon_driver_write(uint8_t reg, uint8_t* data, uint8_t data_len);
static void canon_task_func(void);

static GATE_DRIVER canon_driver = {
	.uid = 0xff01, // canon id
	.major_version = 1,
	.minor_version = 0,
	.read = canon_driver_read,
	.write = canon_driver_write,
	.num_registers = 1,
};

static GATE_TASK canon_task = {
	.task = canon_task_func,
};

static bool fire = 0;

static GATE_RESULT
canon_driver_read(uint8_t reg, uint8_t* data, uint8_t* data_len)
{
	if (!*data_len) {
		return GR_OK;
	}
	
	*data_len = 1;
	*data = fire;

	return GR_OK;
}

static GATE_RESULT
canon_driver_write(uint8_t reg, uint8_t* data, uint8_t data_len)
{	
	if (data_len != 1) {
		return GR_INVALID_ARG;
	}
	
	fire = *data;

	return GR_OK;
}

static void canon_task_func(void)
{
	static bool filter=false,
				state=false, 
				prev_state=false;

	if (fire) {
		motor_on();
	} else {
		// all done, exit
		return;
	}

	bool bs=get_button_state();
	if (filter == bs) {
		state = filter;
	}
	filter = bs;

	if (prev_state && !state) {
		// fire done
		fire = false;
		motor_off();
	}

	prev_state = state;
}

GATE_RESULT init_canon_driver(void)
{
	// Init DDR and PORT
	CANON_DDR = (CANON_DDR | MOTOR_MASK) & ~BUTTON_MASK;
	CANON_PORT = (CANON_PORT | BUTTON_MASK) & ~MOTOR_MASK;

	gate_port_reserve(RESERVE_PORT, RESERVE_MASK, RESERVE_MASK);

	gate_task_register(&canon_task);

	return gate_driver_register(&canon_driver);
}

