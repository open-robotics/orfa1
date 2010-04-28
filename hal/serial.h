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
/** Serial driver
 * @file serial.h
 *
 * @author Vladimir Ermakov <vooon341@gmail.com>
 */

#ifndef SERIAL_H
#define SERIAL_H

#include "serial_lld.h"

#if defined(HAL_HAVE_SERIAL_FILE_DEVICE) || \
	defined(__DOXYGEN__)
/** Serial FILE device
 * Example:
 * @code
 * serial_init(B9600);
 * stdin = stdout = &serial_fdev;
 * @endcode
 */
#define serial_fdev \
	serial_lld_fdev
#endif

/** Send one character
 * @param[in] c character
 */
#define serial_putchar(c) \
	serial_lld_putchar(c)

/** Receive one character
 */
#define serial_getchar() \
	serial_lld_getchar()

/** Check Rxc buffer
 */
#define serial_isempty() \
	serial_lld_isempty()

/** Init serial
 * @param[in] baud baud rate (for example: B115200)
 */
#define serial_init(baud) \
	serial_lld_init(baud)

#endif // SERIAL_H

