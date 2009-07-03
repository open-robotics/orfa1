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

#ifndef OR_AVR_M64_S
#error servo4017: unsupported platform
#endif

#include <avr/io.h>
#include "core/common.h"
#include "core/driver.h"
#include <stdint.h>
#include <stdbool.h>
#include "servo4017.h"

#ifndef NDEBUG
#include <stdio.h>
#endif


static GATE_RESULT driver_read(uint8_t reg, uint8_t* data, uint8_t* data_len);
static GATE_RESULT driver_write(uint8_t reg, uint8_t* data, uint8_t data_len);

static GATE_DRIVER driver = {
	.uid = 0x30,
	.major_version = 1,
	.minor_version = 0,
	.read = driver_read,
	.write = driver_write,
	.num_registers = 2,
};


static GATE_RESULT driver_read(uint8_t reg, uint8_t* data, uint8_t* data_len)
{
	*data_len = 0;
	return GR_OK;
}

static GATE_RESULT driver_write(uint8_t reg, uint8_t* data, uint8_t data_len)
{
	if (reg > 1) {
		return GR_NO_ACCESS;
	}
	if (!reg) {
#ifdef USE_EEPROM
		s4017_save_positions(*data);
#endif
		return GR_OK;
	}
	if (data_len < 3) {
		return GR_INVALID_DATA;
	}
	while (data_len) {
		uint16_t pos = data[2] + (data[1] << 8);
		s4017_set_position(*data, pos);
		data += 3;
		data_len -= 3;
		if (data_len < 3 || data_len > 252) {
			return GR_INVALID_DATA;
		}
	}
	return GR_OK;
}

GATE_RESULT init_servo_driver(void)
{
	s4017_init();
	return gate_driver_register(&driver);
}

