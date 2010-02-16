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

#include "i2cadapter.h"
#include <stdint.h>

#define RESERVED_REGISTERS 2

static uint8_t free_register = RESERVED_REGISTERS;
static GATE_I2CADAPTER* i2cadapters;

static GATE_I2CADAPTER* find_adapter(uint8_t reg)
{
	GATE_I2CADAPTER* adapter = i2cadapters;
	while (adapter) {
		uint8_t num = adapter->num_registers;
		uint8_t start = adapter->start_register;
		if ((reg >= start) && (reg < start+num)) {
			return adapter;
		}
		adapter = adapter->next;
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
	GATE_I2CADAPTER* adapter = find_adapter(reg);
	if (adapter) {
		if (!adapter->read) {
			return GR_NO_ACCESS;
		}
		return adapter->read((reg - adapter->start_register), data, data_len);
	}
	return GR_INVALID_REGISTER;
}

GATE_RESULT gate_register_write(uint8_t reg, uint8_t* data, uint8_t data_len)
{
	GATE_I2CADAPTER* adapter = find_adapter(reg);
	if (adapter) {
		if (!adapter->write) {
			return GR_NO_ACCESS;
		}
		return adapter->write((reg - adapter->start_register), data, data_len);
	}
	return GR_INVALID_REGISTER;
}

GATE_RESULT gate_i2cadapter_register(GATE_I2CADAPTER* adapter)
{
	uint8_t num = adapter->num_registers;
	uint8_t reg = 0x00;
	GATE_RESULT res = GR_OK;

	if (adapter->uid) {
		// if not introspection UID
		// allocate registers
		reg = gate_allocate_registers(num);
		if (!reg) {
			return GR_ALLOCATE_REGISTER;
		}
	}
	adapter->start_register = reg;
	adapter->next = i2cadapters;
	i2cadapters = adapter;
	return res;
}

// -- introspection driver --

static GATE_RESULT intro_read(uint8_t reg, uint8_t* data, uint8_t* data_len);
static GATE_RESULT intro_write(uint8_t reg, uint8_t* data, uint8_t data_len);

static GATE_I2CADAPTER intro_i2cadapter = {
	.uid = 0x0000, // introspection id
	.major_version = 1,
	.minor_version = 0,
	.read = intro_read,
	.write = intro_write,
	.num_registers = 1,
};

static uint8_t intro_num = 0;
static uint8_t intro_len = 0;

static GATE_RESULT intro_read(uint8_t reg, uint8_t* data, uint8_t* data_len)
{
	(void)reg;
	if (!*data_len) {
		return GR_OK;
	}

	if (intro_num > intro_len) {
		intro_num = 1;
	}

	if (intro_num == 0) {
		*data_len = 1;
		*data = intro_len;
		return GR_OK;
	}

	*data_len = 6;
	GATE_I2CADAPTER* adapter = i2cadapters;
	uint8_t i=intro_num;
	while (--i) {
		adapter = adapter->next;
	}

	uint16_t uid = adapter->uid;
	data[0] = (uint8_t) (uid >> 8);
	data[1] = (uint8_t) uid;
	data[2] = adapter->major_version;
	data[3] = adapter->minor_version;
	data[4] = adapter->start_register;
	data[5] = adapter->num_registers;

	// next read — next driver
	++intro_num;

	return GR_OK;
}

static GATE_RESULT intro_write(uint8_t reg, uint8_t* data, uint8_t data_len)
{
	(void)reg;
	if (data_len != 1) {
		return GR_INVALID_ARG;
	}

	if (*data == 0) {
		GATE_I2CADAPTER* adapter = i2cadapters;
		intro_len = 0;
		while (adapter) {
			++intro_len;
			adapter = adapter->next;
		}
	}

	if (*data > intro_len) {
		return GR_INVALID_ARG;
	}

	intro_num = *data;

	return GR_OK;
}

void gate_init_introspection(void)
{
	gate_i2cadapter_register(&intro_i2cadapter);
}

