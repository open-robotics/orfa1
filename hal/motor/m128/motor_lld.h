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
/** RoboMD2 Motor driver
 * @file motor_lld.h
 *
 * @author Andrey Demenev
 * @author Vladimir Ermakov <vooon341@gmail.com>
 */

#ifndef MOTORLLD_H
#define MOTORLLD_H

#include <avr/io.h>
#include <stdint.h>
#include <stdbool.h>

/**
 * @ingroup MotorHal
 *
 * @{
 */

#define DIR_PORT   PORTC
#define DIR_DDR    DDRC
#define DIR_MASK0  _BV(PC7)
#define DIR_MASK1  _BV(PC6)
#define DIR_MASK   (DIR_MASK0|DIR_MASK1)

#define PWM_OC0    OCR1A
#define PWM_OC1    OCR1B
#define PWM_PORT   PORTB
#define PWM_DDR    DDRB
#define PWM_MASK0  _BV(PB5) // OC1A
#define PWM_MASK1  _BV(PB6) // OC1B
#define PWM_MASK   (PWM_MASK0|PWM_MASK1) // avr/iom128.h OC1A, OC1B

// Timer1 OC1A, OC1B config
#define COM_PWM0           ((1<<COM1A1)|(0<<COM1A0))
#define COM_PWM1           ((1<<COM1B1)|(0<<COM1B0))
#define ATTACH_PWM_PIN(ch)  TCCR1A |= COM_PWM##ch
#define DETACH_PWM_PIN(ch)  TCCR1A &= ~COM_PWM##ch

/** Set motor direction
 * @param[in] ch channel [0, 1]
 * @param[in] value firward/backward [false/true]
 */
#define motor_lld_set_direction(ch, value) \
	do {                                   \
		if (value)                         \
			DIR_PORT |= DIR_MASK##ch;      \
		else                               \
			DIR_PORT &= ~DIR_MASK##ch;     \
	} while(0)

/** Set motor PWM
 * @param[in] ch channel [0, 1]
 * @param[in] value PWM
 */
#define motor_lld_set_pwm(ch, value)   \
	do {                               \
		if (value) {                   \
			ATTACH_PWM_PIN(ch);        \
		} else {                       \
			DETACH_PWM_PIN(ch);        \
			PWM_PORT &= ~PWM_MASK##ch; \
		}                              \
		PWM_OC##ch = (uint16_t) value; \
	} while(0)

/** Get motor direction
 * @param[in] ch channel [0, 1]
 */
#define motor_lld_get_direction(ch) \
	((DIR_PORT & DIR_MASK##ch) ? true : false)

/** Get motor PWM
 * @param[in] ch channel [0, 1]
 */
#define motor_lld_get_pwm(ch) \
	((uint8_t) PWM_OC##ch)

/** Init motor driver
 */
void motor_lld_init(void);

#endif // MOTORLLD_H

