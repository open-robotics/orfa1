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

#include "core/common.h"
#include "core/ports.h"
#include "core/driver.h"
#include <avr/io.h>

#include "ports_i2c.h"

static GATE_RESULT
ports_i2cadapter_read(uint8_t reg, uint8_t* data, uint8_t* data_len);
static GATE_RESULT
ports_i2cadapter_write(uint8_t reg, uint8_t* data, uint8_t data_len);

static GATE_PORT ports[] = {
	GATE_PORT_SPECS
};

static GATE_DRIVER ports_i2cadapter = {
	.uid = GATE_PORT_UID,
	.major_version = 1,
	.minor_version = 1,
	.read = ports_i2cadapter_read,
	.write = ports_i2cadapter_write,
	.num_registers = GATE_NUM_PORTS * 2,
};

static GATE_RESULT
ports_i2cadapter_read(uint8_t reg, uint8_t* data, uint8_t* data_len)
{
	if (reg >=  GATE_NUM_PORTS) {
		return GR_NO_ACCESS;
	}
	if (!*data_len) {
		return GR_OK;
	}
	
	GATE_RESULT res = gate_port_read(reg, data);
	if (res == GR_OK) {
		*data_len = 1;
	}
	return res;
}

static GATE_RESULT
ports_i2cadapter_write(uint8_t reg, uint8_t* data, uint8_t data_len)
{
	uint8_t mask=0xFF;
	uint8_t port;
	GATE_RESULT res=GR_OK;

	if (data_len == 0) {
		return GR_OK;
	}

	if (reg >= GATE_NUM_PORTS) {
		port = reg - GATE_NUM_PORTS;
	} else {
		port = reg;
	}

	if (data_len > 1) {
		mask = data[1];
	}
		
	if (reg < GATE_NUM_PORTS) {
		res = gate_port_write(port, mask, data[0]);
	} else {
		res = gate_port_config(port, mask, data[0]);
	}

	return res;
}

// Autoload
MODULE_INIT(ports_i2cadapter)
{
	uint8_t i;
	for (i = 0; i < GATE_NUM_PORTS; i++) {
		if (gate_port_register(i, ports+i) != GR_OK)
			return;
	}

	gate_driver_register(&ports_i2cadapter);
}

