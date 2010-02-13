/*
 *  ORFA -- Open Robotics Firmware Architecture
 *
 *  Copyright (c) 2009-2010 Vladimir Ermakov, Andrey Demenev, Anton Botov
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
/** Servo command low level driver
 * @file servo_cmd_lld.c
 *
 * @author Andrey Demenev
 * @author Anton Botov
 * @author Vladimir Ermakov
 */

#include <avr/io.h>
#include <avr/interrupt.h>

#include "servo_cmd_lld.h"
#include "hal/servo.h"

/// Debug print
#ifndef NDEBUG
#include <stdio.h>
#define debug(...) printf(__VA_ARGS__)
#else
#define debug(...)
#endif

#define ITERATION_STEP 10

static uint16_t servo_pos[SERVO_LEN];
static uint16_t servo_start[SERVO_LEN];
static uint16_t servo_target[SERVO_LEN];
static uint16_t total_time, time_now;

void servo_lld_cmd_init(void)
{
	// Set timer 2 for iterator
	// Frequency 100Hz (7372800/1024/72=100)
	// CTC mode, Prescaler 1/1024
	OCR2 = 72;
	TCCR2 = _BV(WGM21) | _BV(CS22) | _BV(CS20);
	TIMSK |= _BV(OCIE2);
}

bool servo_lld_is_done(void)
{
	return (total_time == 0);
}

ISR(SIG_OUTPUT_COMPARE2) {
	if (total_time == 0)
		return;

	if (total_time != time_now && total_time-time_now < ITERATION_STEP)
		total_time = time_now + ITERATION_STEP;

	for (uint8_t i=0; i<SERVO_LEN; i++)
		if (servo_target[i] != 0) {
			int32_t tmp = servo_target[i];
			tmp -= servo_start[i];
			tmp *= time_now;
			tmp /= total_time;
			tmp += servo_start[i];
			servo_pos[i] = tmp;
			//debug("s4[%d]=%d\n",i,servo_pos[i]);
			servo_set_position(i, servo_pos[i]);
		}

	time_now += ITERATION_STEP;
	if (time_now > total_time) {
		for (uint8_t i=0; i<SERVO_LEN; i++)
			if (servo_target[i] != 0) {
				servo_target[i] = 0;
			}
		total_time = 0;
	}
}

void servo_lld_command(uint16_t time,
		uint16_t *_servo_target,
		uint16_t *_servo_maxspeed)
{
	// Stop current command
	total_time = 0;

	debug("newcmd\n");

	uint16_t maxTime=time;
	uint32_t dx=0;
	for (uint8_t i=0; i<SERVO_LEN; i++)
		if (_servo_target[i] != 0 && _servo_maxspeed[i] !=0) {
			if (_servo_target[i] > servo_pos[i]) {
				dx = _servo_target[i] - servo_pos[i];
			} else {
				dx = servo_pos[i] - _servo_target[i];
			}
			dx *= 1000;
			dx /= _servo_maxspeed[i];
			if (dx > maxTime)
				maxTime=dx;
		}

	debug("time2go=%d\n", maxTime);

	// Load new cmd to iterator variables
	for (uint8_t i=0; i<SERVO_LEN; i++) {
		servo_start[i] = servo_pos[i];
		servo_target[i] = _servo_target[i];
		if (servo_target[i] != 0) {
			debug("st[%d]=%d=>%d\n", i, servo_pos[i], servo_target[i]);
		}
	}

	total_time = maxTime;
	time_now = 0;
	if (total_time == 0)
		total_time = ITERATION_STEP;
}

