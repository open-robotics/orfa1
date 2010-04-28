/*
 *  ORFA -- Open Robotics Firmware Architecture
 *
 *  Copyright (c) 2010 Vladimir Ermakov, Andrey Demenev
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
/** HEX converters
 * @file hex.c
 *
 * @author Vladimir Ermakov <vooon341@gmail.com>
 */

#include "hex.h"

/* Convert a 4-bit integer value to its ASCII representation.
 */
char itox(uint8_t c)
{
	char ret;

	if (c <= 0x09) {
		ret = '0' + c;
	} else if (c <= 0x0f) {
		ret = 'A' + c - 10;
	} else {
		ret = 'X';
	}
	return ret;
}

/* Convert hex to int8 (0 — 15)
 */
int8_t xtoi(uint8_t c)
{
	if ((c >= '0')&&(c <= '9')) {
		return c - '0';
	} else if ((c >= 'A')&&(c <= 'F')) {
		return c - 'A' + 10;
	} else {
		return -1;
	}
}

