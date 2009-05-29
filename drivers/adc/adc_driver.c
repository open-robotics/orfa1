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

#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "core/common.h"
#include "core/driver.h"
#include "core/ports.h"
#include "adc_driver.h"
#include "serialgate/common.h"


/*
 * ADC configuration register
 *
 * Bits 0..2
 *     00     - External reference
 *     01     - AVCC reference
 *     10, 11 - Internal reference
 * 
 * Bit 3
 *     0      -  8-bit
 *     1      - 11-bit
 */
#define ADC_CONFIG_REG 0

#define ADC_DATA_REG 1


uint8_t conversion_channel = 0xFF;
uint8_t conversion_mask;
uint8_t config;
uint16_t result[8];
uint8_t mask;

static uint8_t read_channel;


static GATE_RESULT driver_read(uint8_t reg, uint8_t* data, uint8_t* data_len);
static GATE_RESULT driver_write(uint8_t reg, uint8_t* data, uint8_t data_len);

static GATE_DRIVER driver = {
	.uid = 0x40,
	.major_version = 1,
	.minor_version = 0,
	.read = driver_read,
	.write = driver_write,
	.num_registers = 2,
};

static void reconfigure_adc(uint8_t new_mask)
{
	GATE_PORT* port = find_port(GATE_ADC_PORT_NUMBER);
	if (port) {
		uint8_t changed_mask = mask ^ new_mask;
		gate_port_reserve(GATE_ADC_PORT_NUMBER, changed_mask, new_mask);
		gate_port_reserve(GATE_ADC_PORT_NUMBER, mask, mask);
	}
	if (new_mask) {
		// ADC on
		GATE_ADC_DDR &= ~mask;
		uint8_t admux = ADMUX & ~ (_BV(ADLAR) | (3 << REFS0));
		admux |= (config & 0x03) << REFS0;
		if (!(config & 0x04)){
			admux |= _BV(ADLAR);
		}
		ADMUX = admux;
	} else {
		// ADC off
		ADCSRA = _BV(ADIF);
	}
	if (!mask && new_mask) {
		conversion_channel = 0xFF;
		ADCSRA = _BV(ADEN) | _BV(ADIE) | (5 << ADPS0) | _BV(ADSC) | _BV(ADIF);
	}
	mask = new_mask;
}

static GATE_RESULT driver_read(uint8_t reg, uint8_t* data, uint8_t* data_len)
{
	if (reg != ADC_DATA_REG) {
		return GR_NO_ACCESS;
	}
	uint8_t delta = (config & 0x04) ? 2 : 1;
	uint8_t bytes_written = 0;
	while ((read_channel < 8) && (bytes_written + delta < *data_len)) {
		if (config & 0x04) {
			data[0] = result[read_channel] >> 8;
			data[1] = result[read_channel] & 0xFF;
			data += 2;
			bytes_written += 2;
		} else {
			*data = result[read_channel] & 0xFF;
			data++;
			bytes_written ++;
		}
		read_channel++;
	}
	*data_len = bytes_written;
	return GR_OK;
}

static GATE_RESULT driver_write(uint8_t reg, uint8_t* data, uint8_t data_len)
{
	if (!data_len) return GR_INVALID_DATA;

	if (reg == ADC_CONFIG_REG) {
		if (data_len < 2) return GR_INVALID_DATA;
		config = data[0];
		if ((config & 0x03) == 0x02) {
			config |= 0x03;
		}
		reconfigure_adc(data[1]);
	} else {
		read_channel = *data;
	}

	return GR_OK;
}

GATE_RESULT init_adc_driver(void)
{
	return gate_driver_register(&driver);
}


ISR(SIG_ADC)
{
	uint8_t low = ADCL;
	uint8_t high = ADCH;

	if (conversion_channel != 0xFF) {
		if (config & 0x04) {
			result[conversion_channel] = (high << 8) + low;
		} else {
			result[conversion_channel] = high;
		}
		conversion_channel++;
		conversion_channel &= 0x07;
		conversion_mask <<= 1;
		if (!conversion_mask) {
			conversion_mask = 0x01;
		}
	} else {
		conversion_channel = 0;
		conversion_mask = 0x01;
	}
	while (!(conversion_mask & mask)) {
		conversion_mask <<= 1;
		conversion_channel++;
	}
	ADMUX = (ADMUX & ~0x07) | conversion_channel;
	ADCSRA = _BV(ADEN) | _BV(ADIE) | (5 << ADPS0) | _BV(ADSC);
}


