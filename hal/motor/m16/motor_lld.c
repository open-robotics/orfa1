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
 * @file motor_lld.c
 * @author Vladimir Ermakov
 *
 * @todo use ATOMIC_BLOCK()
 */

#include <avr/io.h>
#include "motor_lld.h"

void motor_lld_init(void)
{
	// init DIR i/o lines
	DIR_PORT0 &= ~DIR_MASK0;
	DIR_DDR0 |= DIR_MASK0;
	DIR_PORT1 &= ~DIR_MASK1;
	DIR_DDR1 |= DIR_MASK1;

	// init PWM i/o lines
	PWM_PORT &= ~PWM_MASK;
	PWM_DDR |= PWM_MASK;

	// clear pwm registers
	PWM_OC0 = 0;
	PWM_OC1 = 0;

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
}

