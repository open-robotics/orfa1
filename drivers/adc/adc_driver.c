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
/** ADC driver
 * @file adc_driver.c
 *
 * @author Andrey Demenev
 * @author Vladimir Ermakov
 */

#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "core/common.h"
#include "core/driver.h"
#include "core/ports.h"
#include "core/scheduler.h"
#include "adc_driver.h"
#include "serialgate/common.h"


/**
 * @ingroup Drivers
 * @defgroup ADC ADC driver
 *
 * UID: 0x0040
 *
 * @{
 */

/** ADC configuration register
 * @code
 * Bits 0..2
 *     00     - External reference
 *     01     - AVCC reference
 *     10, 11 - Internal reference
 * 
 * Bit 3
 *     0      -  8-bit
 *     1      - 11-bit
 * @endcode
 */
#define ADC_CONFIG_REG 0

#define ADC_DATA_REG 1

#ifdef ADC_ISR
#define ADC_INTERRUPT_MASK _BV(ADIE)
#define ADC_VOLATILE volatile
#else
#define ADC_INTERRUPT_MASK 0
#define ADC_VOLATILE
#endif

// ISR data
static ADC_VOLATILE uint8_t conversion_channel = 0xFF;
static ADC_VOLATILE uint8_t conversion_mask;
static ADC_VOLATILE uint8_t config;
static ADC_VOLATILE uint16_t result[8];
static ADC_VOLATILE uint8_t mask;

// driver data
static uint8_t read_channel;

static GATE_RESULT adc_driver_read(uint8_t reg, uint8_t* data, uint8_t* data_len);
static GATE_RESULT adc_driver_write(uint8_t reg, uint8_t* data, uint8_t data_len);

static GATE_DRIVER adc_driver = {
	.uid = 0x0040,
	.major_version = 1,
	.minor_version = 0,
	.read = adc_driver_read,
	.write = adc_driver_write,
	.num_registers = 2,
};

#ifndef ADC_ISR
static void adc_task(void);

static GATE_TASK task = {
	&adc_task,
	0,
};

#endif

static void reconfigure_adc(uint8_t new_mask)
{
	debug("# adc->reconfigure_adc(0x%X)\n# adc:: mask=0x%X\n", new_mask, mask);

	GATE_PORT* port = find_port(GATE_ADC_PORT_NUMBER);
	if (port) {
		uint8_t changed_mask = mask ^ new_mask;
		gate_port_reserve(GATE_ADC_PORT_NUMBER, changed_mask, new_mask);
		gate_port_reserve(GATE_ADC_PORT_NUMBER, mask, mask);
	}

	if (new_mask) {
		// ADC on
		GATE_ADC_DDR &= ~new_mask;
		uint8_t admux = ADMUX & ~ (_BV(ADLAR) | (3 << REFS0));
		
		admux |= (config & 0x03) << REFS0;
		if (!(config & 0x04)) {
			// 10-bit
			admux |= _BV(ADLAR);
		}

		ADMUX = admux;
	} else {
		// ADC off
		ADCSRA = _BV(ADIF);
	}

	if (!mask && new_mask) {
		mask = new_mask;
		conversion_channel = 0xFF;
		ADCSRA = _BV(ADEN) | ADC_INTERRUPT_MASK | (5 << ADPS0) | _BV(ADSC) | _BV(ADIF);
	} else {
		mask = new_mask;
	}
}

static GATE_RESULT adc_driver_read(uint8_t reg, uint8_t* data, uint8_t* data_len)
{
	debug("# adc->read(%i, buf, %i)\n", reg, *data_len);

	if (reg != ADC_DATA_REG) {
		return GR_NO_ACCESS;
	}

	if (!*data_len) {
		return GR_OK;
	}

	if (config & 0x04) {
		// 10-bit
		data[0] = result[read_channel] >> 8;
		data[1] = result[read_channel] & 0xFF;
		*data_len = 2;
	} else {
		// 8-bit
		*data = result[read_channel] & 0xFF;
		*data_len = 1;
	}

	// circle read
	if (read_channel++ >= 8) {
		read_channel = 0;
	}

	return GR_OK;
}

static GATE_RESULT adc_driver_write(uint8_t reg, uint8_t* data, uint8_t data_len)
{
	debug("# adc->write(%i, buf, %i)\n", reg, data_len);

	if (!data_len) {
		return GR_INVALID_DATA;
	}

	if (reg == ADC_CONFIG_REG) {
		if (data_len < 2) {
			return GR_INVALID_DATA;
		}

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

#ifdef ADC_ISR
ISR(ADC_vect)
#else
static void adc_task(void)
#endif
{
#ifndef ADC_ISR
	if (!(ADCSRA & _BV(ADIF))) return;
	ADCSRA | _BV(ADIF);
#endif
	if (conversion_channel != 0xFF) {
		result[conversion_channel] = ADC;
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

	// find next channel
	for (uint8_t i=0; i<8 && !(conversion_mask & mask); i++) {
		/* debug("# ADC IRQ #%i 0x%X & 0x%X = 0x%X => %i\n", i, 
				conversion_mask, mask, conversion_mask & mask, 
				conversion_channel); */
		conversion_channel++;
		conversion_channel &= 0x07;
		conversion_mask <<= 1;
		if (!conversion_mask) {
			conversion_mask = 0x01;
		}
	}

	// set channel and run conversion
	ADMUX = (ADMUX & ~0x07) | conversion_channel;
	ADCSRA = _BV(ADEN) | ADC_INTERRUPT_MASK | (5 << ADPS0) | _BV(ADSC);
}

// module autoload
MODULE_INIT(adc_driver)
{
	gate_driver_register(&adc_driver);
#ifndef ADC_ISR
	gate_task_register(&task);
#endif
}

