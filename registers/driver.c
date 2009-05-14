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

#include "driver.h"
#include <stdint.h>

#define RESERVED_REGISTERS 2

static uint8_t free_register = RESERVED_REGISTERS;
static GATE_DRIVER* drivers;

static GATE_DRIVER* find_driver(uint8_t reg)
{
	GATE_DRIVER* driver = drivers;
	while (driver) {
		uint8_t i = driver->num_registers;
		uint8_t* cur = driver->registers;
		while (i) {
			if (*cur == reg) {
				return driver;
			}
			i--;
			cur++;
		}
		driver = driver->next;
	}
	return 0;
}

GATE_RESULT gate_register_read(uint8_t reg, uint8_t* data, uint8_t* data_len)
{
	GATE_DRIVER* driver = find_driver(reg);
	if (driver) {
		if (!driver->read) {
			return GR_NO_ACCESS;
		}
		return driver->read(reg, data, data_len);
	}
	return GR_INVALID_REGISTER;
}

GATE_RESULT gate_register_write(uint8_t reg, uint8_t* data, uint8_t data_len)
{
	GATE_DRIVER* driver = find_driver(reg);
	if (driver) {
		if (!driver->write) {
			return GR_NO_ACCESS;
		}
		return driver->write(reg, data, data_len);
	}
	return GR_INVALID_REGISTER;
}

GATE_RESULT gate_driver_register(GATE_DRIVER* driver)
{
	uint8_t i = driver->num_registers;
	while (i) {
		if (find_driver(driver->registers[i])) {
			return GR_DUPLICATE_REGISTER;
		}
		i--;
	}
	GATE_RESULT res = GR_OK;
	if (driver->init) {
		res = driver->init();
	}
	if (res == GR_OK) {
		driver->next = drivers;
		drivers = driver;
	}
	return res;
}

uint8_t gate_allocate_register(void)
{
	if (!free_register) {
		return 0;
	}
	return free_register++;
}

