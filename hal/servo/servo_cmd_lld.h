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
/** Servo command low level driver header
 * @file servo/servo_cmd_lld.h
 *
 * @author Anton Botov
 * @author Vladimir Ermakov
 */

#ifndef SERVOCMD_H
#define SERVOCMD_H

#include <stdint.h>
#include <stdbool.h>

/** Check that command is done
 */
bool servo_lld_is_done(void);

/** New command
 */
void servo_lld_command(uint16_t time,
		uint16_t *_servo_target, uint16_t *_servo_maxspeed);

/** Init servo command
 */
void servo_lld_cmd_init(void);

#if defined(HAL_SEVRO_NTIM2) || defined(__DOXYGEN__)
/** Servo commnad periodic
 */
void servo_lld_loop(void);
#endif

#endif // SERVO4017_H

