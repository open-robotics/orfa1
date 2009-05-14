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

#include "common.h"
#include "ports.h"
#include "driver.h"
#include <avr/io.h>
#include <string.h>


static GATE_RESULT spi_driver_read(uint8_t reg, uint8_t* data, uint8_t* data_len);
static GATE_RESULT spi_driver_write(uint8_t reg, uint8_t* data, uint8_t data_len);

#define SPI_BUF_SIZE 64

#define DATA_REG   0x50
#define CONFIG_REG 0x51

typedef struct {
	GATE_PORT* port;
	uint8_t mask;   
} SPI_CONTROL_PIN;

static SPI_CONTROL_PIN control_pin[4];
static uint8_t buf[SPI_BUF_SIZE];
static uint8_t buf_len;

static uint8_t registers[] = {
	CONFIG_REG,
	DATA_REG,
};

static GATE_DRIVER driver = {
	.uid = 0x1000, // spi id
	.read = spi_driver_read,
	.write = spi_driver_write,
	.registers = registers,
	.num_registers = NUM_ELEMENTS(registers),
};

static GATE_RESULT spi_driver_read(uint8_t reg, uint8_t* data, uint8_t* data_len)
{
	if (reg >= CONFIG_REG) {
		return GR_NO_ACCESS;
	}
	if (*data_len > buf_len) {
		*data_len = buf_len;
	}
	if (!*data_len) {
		return GR_OK;
	}
	memcpy(data, buf, *data_len);
	return GR_OK;
}

static GATE_RESULT spi_write(uint8_t* data, uint8_t data_len)
{
	if (!data_len < 1) {
		return GR_INVALID_DATA;
	}
	data_len--;
	buf_len = 0;
	uint8_t channel = *data;
	if (channel >= NUM_ELEMENTS(control_pin)) {
		return GR_INVALID_DATA;
	}
	if (!control_pin[channel].port) {
		return GR_INVALID_DATA;
	}
	*(control_pin[channel].port->PORT) &= ~control_pin[channel].mask;
	while (data_len) {
		SPDR = data[buf_len+1];
		while (!(SPSR & _BV(SPIF))) ;
		if (buf_len < SPI_BUF_SIZE) {
			buf[buf_len] = SPDR;
		}
		buf_len++;
		data_len--;
	}
	*(control_pin[channel].port->PORT) |= control_pin[channel].mask;
	return GR_OK;
}

static GATE_RESULT spi_config(uint8_t* data, uint8_t data_len)
{
	if (data_len < 2) {
		return GR_INVALID_DATA;
	}
	uint8_t channel = data[0];
	if (channel == 0xFF) {
		uint8_t value = SPCR;
		value &= ~0x03;
		value |= (data[2] & 0x03);
		SPCR = value;
		value = SPSR;
		value &= ~0x01;
		value |= ((data[2] >> 2)  & 0x01);
		SPSR = value;
		return GR_OK;
	}
	if (channel >= NUM_ELEMENTS(control_pin)) {
		return GR_INVALID_DATA;
	}
	if (data_len < 3) {
		return GR_INVALID_DATA;
	}
	if (data[2] > 7) {
		return GR_INVALID_DATA;
	}
	uint8_t mask = 1 << data[2];
	uint8_t res = gate_port_reserve(data[1], mask, mask);
	if (res != GR_OK) {
		return res;
	}
	GATE_PORT* port = find_port(data[1]);
	control_pin[channel].port = port;
	control_pin[channel].mask = mask;
	*(port->PORT) |= mask;
	*(port->DDR) |= mask;
	SPCR |= _BV(MSTR) | _BV(SPE);
	return GR_OK;
}

static GATE_RESULT spi_driver_write(uint8_t reg, uint8_t* data, uint8_t data_len)
{
	if (reg < CONFIG_REG) {
		return spi_write(data, data_len);
	}
	return spi_config(data, data_len);
}

GATE_RESULT init_spi_driver(void)
{
	return gate_driver_register(&driver);
}


