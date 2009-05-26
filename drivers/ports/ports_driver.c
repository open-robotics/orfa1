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
#include "ports_driver.h"

static GATE_RESULT port_driver_read(uint8_t reg, uint8_t* data, uint8_t* data_len);
static GATE_RESULT port_driver_write(uint8_t reg, uint8_t* data, uint8_t data_len);

static GATE_PORT ports[] = {
	GATE_PORT_SPECS
};

static GATE_DRIVER driver = {
	.uid = GATE_PORT_UID,
	.major_version = 1,
	.minor_version = 0,
	.read = port_driver_read,
	.write = port_driver_write,
	.num_registers = GATE_NUM_PORTS * 2,
};

static GATE_RESULT port_driver_read(uint8_t reg, uint8_t* data, uint8_t* data_len)
{
	if (reg >=  GATE_NUM_PORTS) {
		return GR_NO_ACCESS;
	}
	if (!*data_len) {
		return GR_OK;
	}
	
	GATE_RESULT res;
	res = gate_port_read(reg, data);
	if (res == GR_OK) {
		*data_len = 1;
	}
	return res;
}

static GATE_RESULT port_driver_write(uint8_t reg, uint8_t* data, uint8_t data_len)
{
	uint8_t i = 0;
	uint8_t port;
	
	if (reg >= GATE_NUM_PORTS) {
		port = reg - GATE_NUM_PORTS;
	} else {
		port = reg;
	}

	while (i < data_len) {
		GATE_RESULT res;
		if (reg < GATE_NUM_PORTS) {
			res = gate_port_write(port, 0xFF, data[i]);
		} else {
			res = gate_port_config(port, 0xFF, data[i]);
		}
		if (res != GR_OK) {
			return res;
		}
		i ++;
	}
	return GR_OK;
}

GATE_RESULT init_ports_driver(void)
{
	uint8_t i;
	for (i = 0; i < GATE_NUM_PORTS; i++) {
		GATE_RESULT res;
		res = gate_port_register(i, ports+i);
		if (res != GR_OK) return res;
	}
	return gate_driver_register(&driver);
}

