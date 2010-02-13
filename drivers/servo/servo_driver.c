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
/** Servo driver
 * @file servo_driver.c
 *
 * @author Vladimir Ermakov
 */

#include "servo_driver.h"

static GATE_RESULT
servo_driver_read(uint8_t reg, uint8_t* data, uint8_t* data_len);
static GATE_RESULT
servo_driver_write(uint8_t reg, uint8_t* data, uint8_t data_len);

static GATE_DRIVER servo_driver = {
	.uid = SERVO_UID,
	.major_version = 1,
	.minor_version = SERVO_MINOR,
	.read = servo_driver_read,
	.write = servo_driver_write,
	.num_registers = 2,
};

static GATE_RESULT
servo_driver_read(uint8_t reg, uint8_t* data, uint8_t* data_len)
{
	*data_len = 0;
	return GR_OK;
}

static GATE_RESULT
servo_driver_write(uint8_t reg, uint8_t* data, uint8_t data_len)
{
	if (reg > 1) {
		return GR_NO_ACCESS;
	}
	if (!reg) {
		return GR_OK;
	}
	if (data_len < 3) {
		return GR_INVALID_DATA;
	}
	while (data_len) {
		servo_set_position(*data, (data[1]<<8)|data[2]);
		data += 3;
		data_len -= 3;
		if (data_len < 3 || data_len > 252) {
			return GR_INVALID_DATA;
		}
	}
	return GR_OK;
}

MODULE_INIT(servo_driver)
{
	servo_init();
	gate_driver_register(&servo_driver);
}

