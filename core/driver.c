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
#include "common.h"
#include <stdint.h>

#define RESERVED_REGISTERS 2

static uint8_t free_register = RESERVED_REGISTERS;
static GATE_DRIVER* drivers;

static GATE_DRIVER* find_driver(uint8_t reg)
{
	GATE_DRIVER* driver = drivers;
	reg &= ~0x80; // mask high bit
	while (driver) {
		uint8_t num = driver->num_registers;
		uint8_t start = driver->start_register;
		if ((reg >= start) && (reg < start+num)) {
			return driver;
		}
		driver = driver->next;
	}
	return 0;
}

static uint8_t gate_allocate_registers(uint8_t count)
{
	uint8_t reg = 0;
	if (!free_register) {
		return 0;
	}
	if (!(free_register + count)) {
		return 0;
	}
	reg = free_register;
	free_register += count;
	return reg;
}

GATE_RESULT gate_register_read(uint8_t reg, uint8_t* data, uint8_t* data_len)
{
	GATE_DRIVER* driver = find_driver(reg);
	if (driver) {
		if (!driver->read) {
			return GR_NO_ACCESS;
		}
		return driver->read((reg-driver->start_register), data, data_len);
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
		return driver->write((reg-driver->start_register), data, data_len);
	}
	return GR_INVALID_REGISTER;
}

GATE_RESULT gate_driver_register(GATE_DRIVER* driver)
{
	uint8_t num = driver->num_registers;
	uint8_t reg = 0x00;
	GATE_RESULT res = GR_OK;

	if (driver->uid) {
		// if not introspection UID
		// allocate registers
		reg = gate_allocate_registers(num);
		if (!reg) {
			return GR_ALLOCATE_REGISTER;
		}
	}
	driver->start_register = reg;
	if (driver->init) {
		res = driver->init();
	}
	if (res == GR_OK) {
		driver->next = drivers;
		drivers = driver;
	}
	return res;
}

// introspection driver -------------------------------------------------------

static GATE_RESULT idriver_read(uint8_t reg, uint8_t* data, uint8_t* data_len);
static GATE_RESULT idriver_write(uint8_t reg, uint8_t* data, uint8_t data_len);

static GATE_DRIVER idriver = {
	.uid = 0x0000, // introspection id
	.major_version = 1,
	.minor_version = 0,
	.read = idriver_read,
	.write = idriver_write,
	.num_registers = 1,
};

static uint8_t idriver_num = 0;
static uint8_t idriver_len = 0;

static GATE_RESULT idriver_read(uint8_t reg, uint8_t* data, uint8_t* data_len)
{
	(void)reg;
	if (!*data_len) {
		return GR_OK;
	}
	
	if (idriver_num > idriver_len) {
		idriver_num = 1;
	}

	if (idriver_num == 0) {
		*data_len = 1;
		*data = idriver_len;
		return GR_OK;
	}

	*data_len = 6;
	GATE_DRIVER* driver = drivers;
	uint8_t i=idriver_num;
	while (--i) {
		driver = driver->next;
	}
	
	if (driver) {
		uint16_t uid = driver->uid;
		data[0] = (uint8_t) (uid >> 8);
		data[1] = (uint8_t) uid;
		data[2] = driver->major_version;
		data[3] = driver->minor_version;
		data[4] = driver->start_register;
		data[5] = driver->num_registers;
	}

	// next read — next driver
	++idriver_num;

	return GR_OK;
}

static GATE_RESULT idriver_write(uint8_t reg, uint8_t* data, uint8_t data_len)
{
	(void)reg;
	if (data_len != 1) {
		return GR_INVALID_ARG;
	}

	if (*data == 0) {
		GATE_DRIVER* driver = drivers;
		idriver_len = 0;
		while (driver) {
			++idriver_len;
			driver = driver->next;
		}
	}

	if (*data > idriver_len) {
		return GR_INVALID_ARG;
	}

	idriver_num = *data;

	return GR_OK;
}

void gate_init_introspection(void)
{
	gate_driver_register(&idriver);
}

