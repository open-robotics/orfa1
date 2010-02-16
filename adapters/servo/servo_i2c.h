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
/** Servo I2C adapter
 * @file servo_i2c.h
 *
 * @author Andrey Demenev
 * @author Vladimir Ermakov
 */

#ifndef SERVO_DRIVER_H
#define SERVO_DRIVER_H

/**
 * @ingroup Drivers
 * @defgroup Servo Servo driver
 *
 * @{
 */

/// Servo config. NOT USED
#define SERVO_CONF 0x00
/// Servo control register
#define SERVO 0x01

#ifdef OR_AVR_M128_S

#define SERVO_UID   0x30
#define SERVO_MAJOR 1
#define SERVO_MINOR 0

#elif defined(OR_AVR_M32_D)

#define SERVO_UID   0x31
#define SERVO_MAJOR 1
#define SERVO_MINOR 1

#else
#error Unsupported platform
#endif

#include <avr/io.h>
#include "core/i2cadapter.h"

#include "hal/servo.h"

#endif // SERVO_DRIVER_H
