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
/** Servo driver
 * @file hal/servo.h
 *
 * @author Vladimir Ermakov <vooon341@gmail.com>
 */

#ifndef SERVO_H
#define SERVO_H

#include <stdint.h>
#include <stdbool.h>

#include "servo_lld.h"

/**
 * @addtogroup HalServo
 * @{
 */

/** Set servo position
 * @param[in] servo      servo number
 * @param[in] position   servo position in usec
 */
#define servo_set_position(servo, position) \
	servo_lld_set_position(servo, position)

/** Init servo subsystem
 */
#define servo_init() \
	servo_lld_init()

#if defined(HAL_WITH_SERVO_CMD) || defined(__DOXYGEN__)
#include "servo_cmd_lld.h"

#undef servo_init()
#define servo_init()      \
	servo_lld_init();     \
	servo_lld_cmd_init()

/** Check that command is done
 */
#define servo_is_done() \
	servo_lld_is_done()

/** New servo command
 * @param[in] time
 * @param[in] target
 * @param[in] maxspeed
 */
#define servo_command(time, target, maxspeed) \
	servo_lld_command(time, target, maxspeed)

#endif

#endif // SERVO_H

