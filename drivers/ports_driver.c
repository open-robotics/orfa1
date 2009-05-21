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

#include "registers/common.h"
#include "registers/ports.h"
#include "registers/driver.h"
#include <avr/io.h>

static GATE_RESULT port_driver_read(uint8_t reg, uint8_t* data, uint8_t* data_len);
static GATE_RESULT port_driver_write(uint8_t reg, uint8_t* data, uint8_t data_len);

#define DATA_A   0x40
#define DATA_B   0x41
#define DATA_C   0x42
#define DATA_D   0x43

#define CONFIG_A 0x44
#define CONFIG_B 0x45
#define CONFIG_C 0x46
#define CONFIG_D 0x47

static GATE_PORT ports[] = {
	// PORTA
	{
		.PORT = (void*)_SFR_MEM_ADDR(PORTA),
		.PIN = (void*)_SFR_MEM_ADDR(PINA),
		.DDR = (void*)_SFR_MEM_ADDR(DDRA),
	},
	// PORTB
	{
		.PORT = (void*)_SFR_MEM_ADDR(PORTB),
		.PIN = (void*)_SFR_MEM_ADDR(PINB),
		.DDR = (void*)_SFR_MEM_ADDR(DDRB),
		.default_busy_mask = 0xF3,
	},
	// PORTC
	{
		.PORT = (void*)_SFR_MEM_ADDR(PORTC),
		.PIN = (void*)_SFR_MEM_ADDR(PINC),
		.DDR = (void*)_SFR_MEM_ADDR(DDRC),
		.default_busy_mask = 0x03,
	},
	// PORTD
	{
		.PORT = (void*)_SFR_MEM_ADDR(PORTD),
		.PIN = (void*)_SFR_MEM_ADDR(PIND),
		.DDR = (void*)_SFR_MEM_ADDR(DDRD),
		.default_busy_mask = 0xC3,
	},
};

static uint8_t registers[] = {
	CONFIG_A,
	CONFIG_B,
	CONFIG_C,
	CONFIG_D,
	DATA_A,
	DATA_B,
	DATA_C,
	DATA_D,
};

static GATE_DRIVER driver = {
	.uid = 0x0010, // port id
	.major_version = 1,
	.minor_version = 0,
	.read = port_driver_read,
	.write = port_driver_write,
	.registers = registers,
	.num_registers = NUM_ELEMENTS(registers),
};

static GATE_RESULT port_driver_read(uint8_t reg, uint8_t* data, uint8_t* data_len)
{
	if (reg >=  CONFIG_A) {
		return GR_NO_ACCESS;
	}
	if (!*data_len) {
		return GR_OK;
	}
	
	GATE_RESULT res;
	res = gate_port_read(reg - DATA_A, data);
	if (res == GR_OK) {
		*data_len = 1;
	}
	return res;
}

static GATE_RESULT port_driver_write(uint8_t reg, uint8_t* data, uint8_t data_len)
{
	uint8_t i = 0;
	uint8_t port;
	
	if (reg >= CONFIG_A) {
		port = reg - CONFIG_A;
	} else {
		port = reg - DATA_A;
	}

	while (i < data_len) {
		GATE_RESULT res;
		if (reg < CONFIG_A) {
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
	GATE_RESULT res;
	res = gate_port_register(0, ports);
	if (res != GR_OK) return res;
	res = gate_port_register(1, ports+1);
	if (res != GR_OK) return res;
	res = gate_port_register(2, ports+2);
	if (res != GR_OK) return res;
	res = gate_port_register(3, ports+3);
	if (res != GR_OK) return res;
	return gate_driver_register(&driver);
}


