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
 * @file motor.h
 *
 * @author Andrey Demenev
 * @author Vladimir Ermakov <vooon341@gmail.com>
 */

#ifndef MOTOR_H
#define MOTOR_H

#include "motor_lld.h"

/** Set motor direction
 * @param[in] ch channel [0, 1]
 * @param[in] value firward/backward [false/true]
 */
#define motor_set_direction(ch, value) \
	motor_lld_set_direction(ch, value)

/** Set motor PWM
 * @param[in] ch channel [0, 1]
 * @param[in] value PWM
 */
#define motor_set_pwm(ch, value) \
	motor_lld_set_pwm(ch, value)

/** Get motor direction
 * @param[in] ch channel [0, 1]
 */
#define motor_get_direction(ch) \
	motor_lld_get_direction(ch)

/** Get motor PWM
 * @param[in] ch channel [0, 1]
 */
#define motor_get_pwm(ch) \
	motor_lld_get_pwm(ch)

/** Init motor driver
 */
#define motor_init() \
	motor_lld_init()

#endif // MOTOR_H

