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

#include "orc32.h"
#include "common.h"
#include "hal/serial.h"
#include "parser.h"

#include <stdint.h>
#include <stdio.h>

//! error state
static error_code_t error_code=NO_ERROR;

/**
 * Convert a 4-bit integer value to its ASCII representation.
 *
 * @param c the value to convert.
 * @return the converted value or 'X' on error.
 */
static char itox(uint8_t c)
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

void print_error(error_code_t error_code)
{
	putchar('E');
	putchar('R');
	putchar('R');
	putchar('O');
	putchar('R');
	putchar(' ');
	putchar(itox(error_code >> 4));
	putchar(itox(error_code & 0x0f));
	putchar('\n');
}


void orc32_init(void)
{

	serial_init(BAUD);
	stdin = stdout = stderr = &serial_fdev;
}

void orc32_supertask(void)
{
	if(serial_isempty())
		return;

	uint8_t c = getchar();

	parse_cmd(c, &error_code);

	if(error_code != NO_ERROR)
	{
		print_error(error_code);
		error_code = NO_ERROR;
	}
}

