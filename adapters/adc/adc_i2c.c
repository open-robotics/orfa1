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
/** ADC I2C adapter
 * @file adc_i2c.c
 *
 * @author Andrey Demenev
 * @author Vladimir Ermakov
 */

#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "core/driver.h"
#include "core/ports.h"
#include "core/scheduler.h"

#include "adc_i2c.h"

#include "hal/adc.h"

/**
 * @addtogroup ADCAdapter
 * @{
 */

/** ADC configuration register
 * @code
 * Bits 0..1
 *     00     - External reference
 *     01     - AVCC reference
 *     10, 11 - Internal reference
 * 
 * Bit 2
 *     0      -  8-bit
 *     1      - 10-bit
 * @endcode
 */
#define ADC_CONFIG_REG 0

#define ADC_DATA_REG 1

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

#ifdef HAL_ADC_NISR
static GATE_TASK adc_task = {
	.task = adc_loop,
};
#endif

static GATE_RESULT adc_driver_read(uint8_t reg, uint8_t* data, uint8_t* data_len)
{
	debug("# adc->read(%i, buf, %i)\n", reg, *data_len);

	if (reg != ADC_DATA_REG) {
		return GR_NO_ACCESS;
	}

	if (!*data_len) {
		return GR_OK;
	}

	if (adc_is_10bit()) {
		// 10-bit
		data[0] = adc_result[read_channel] >> 8;
		data[1] = adc_result[read_channel] & 0xFF;
		*data_len = 2;
	} else {
		// 8-bit
		*data = adc_result[read_channel] & 0xFF;
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

		adc_config = data[0];
		if ((adc_config & 0x03) == 0x02) {
			adc_config |= 0x03;
		}
		adc_reconfigure(data[1]);

#if 0 // TODO!
		GATE_PORT* port = find_port(GATE_ADC_PORT_NUMBER);
		if (port) {
			uint8_t changed_mask = mask ^ data[1];
			gate_port_reserve(GATE_ADC_PORT_NUMBER, changed_mask, data[1]);
			gate_port_reserve(GATE_ADC_PORT_NUMBER, mask, mask);
		}
#endif

		if (data[1]) {
			GATE_ADC_DDR &= ~data[1];
		}

	} else {
		read_channel = *data;
	}

	return GR_OK;
}

// module autoload
MODULE_INIT(adc_driver)
{
#ifdef HAL_ADC_NISR
	gate_task_register(&adc_task);
#endif
	gate_driver_register(&adc_driver);
}

