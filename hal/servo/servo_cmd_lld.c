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

static uint16_t servo_start[SERVO_LEN];
static uint16_t servo_target[SERVO_LEN];
static uint16_t servo_total_time[SERVO_LEN];
static uint16_t servo_time_left[SERVO_LEN];

void servo_lld_cmd_init(void)
{
#ifndef HAL_SERVO_NTIM
	#ifndef HAL_SERVO_TIM0
	// Set timer 2 for iterator
	// Frequency 100Hz (7372800/1024/72=100)
	// CTC mode, Prescaler 1/1024
	OCR2 = 72;
	TCCR2 = _BV(WGM21) | _BV(CS22) | _BV(CS20);
	TIMSK |= _BV(OCIE2);
	#else
	// Set timer 0 for iteraror
	// Frequency 100 Hz (7372800/1024/72=100)
	// CTC mode, Prescaler 1/1024
	OCR0 = 72;
	TCCR0 = _BV(WGM01) | _BV(CS02) | _BV(CS00);
	TIMSK |= _BV(OCIE0);
	#endif
#endif
}

bool servo_lld_is_done(void)
{
	uint16_t total_time=0;
	for (uint8_t i=0; i<SERVO_LEN; i++)
		if (total_time < servo_time_left[i])
			total_time = servo_time_left[i];
	return total_time == 0;
}

#ifndef HAL_SERVO_NTIM
	#ifndef HAL_SERVO_TIM0
ISR(SIG_OUTPUT_COMPARE2)
	#else
ISR(SIG_OUTPUT_COMPARE0)
	#endif
#else
void servo_lld_loop(void)
#endif
{
	int32_t tmp;

	asm volatile ("sei"); // XXX: Warning!

	for (uint8_t i=0; i<SERVO_LEN; i++)
		if (servo_time_left[i] > 0) {
			if (servo_time_left[i] <= ITERATION_STEP) {
				servo_time_left[i] = 0;
				tmp = servo_target[i];
				debug("# fs %d %d\n", i, tmp);
			} else {
				servo_time_left[i] -= ITERATION_STEP;
				tmp = servo_start[i];
				tmp -= servo_target[i];
				tmp *= servo_time_left[i];
				tmp /= servo_total_time[i];
				tmp += servo_target[i];
			}
			servo_set_position(i, tmp);
		}
}

void servo_lld_command(uint16_t time,
		uint16_t *_servo_target,
		uint16_t *_servo_maxspeed)
{
	uint16_t maxTime = time;

	for (uint8_t i=0; i<SERVO_LEN; i++)
		if (_servo_target[i] != 0 && _servo_maxspeed[i] !=0) {
			uint32_t dx=0;
			uint16_t pos=servo_get_position(i);
			if (_servo_target[i] > pos)
				dx = _servo_target[i] - pos;
			else
				dx = pos - _servo_target[i];
			dx *= 1000;
			dx /= _servo_maxspeed[i];
			if (dx > maxTime)
				maxTime = dx;
		}

	if (maxTime < ITERATION_STEP)
		maxTime = ITERATION_STEP;

	debug("# time2go=%d\n", maxTime);

	//Load new cmd to iterator variables
	for (uint8_t i=0; i<SERVO_LEN; i++)
		if(_servo_target[i] != 0) {
			uint16_t pos=servo_get_position(i);
			debug("# st[%d]=%d->%d\n", i, pos, _servo_target[i]);
			servo_start[i] = pos;
			servo_target[i] = _servo_target[i];
			servo_total_time[i] = maxTime;
			servo_time_left[i] = maxTime;
		}
}

