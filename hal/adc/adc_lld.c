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
/** ADC low level driver
 * @file adc_lld.c
 *
 * @author Andrey Demenev
 * @author Vladimir Ermakov
 */

#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "adc_lld.h"

#ifndef HAL_ADC_NISR
#define ADC_INTERRUPT_MASK _BV(ADIE)
#else
#define ADC_INTERRUPT_MASK 0
#endif

// extern data
ADC_VOLATILE uint8_t adc_lld_config=0x04;
ADC_VOLATILE uint16_t adc_lld_result[ADC_LEN];
// ISR data
static ADC_VOLATILE uint8_t conversion_channel = 0xFF;
static ADC_VOLATILE uint8_t conversion_mask;
static ADC_VOLATILE uint8_t mask;

uint16_t adc_lld_get_result(uint8_t channel)
{
	if(channel>7)return 0;
	return adc_lld_result[channel];
}

uint8_t adc_lld_get_mask(void)
{
	return mask;
}

void adc_lld_reconfigure(uint8_t new_mask)
{
	if (new_mask) {
		// ADC on
		uint8_t admux = ADMUX & ~ (_BV(ADLAR) | (3 << REFS0));
		
		admux |= (adc_lld_config & 0x03) << REFS0;
		if (!(adc_lld_config & 0x04)) {
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

#ifndef HAL_ADC_NISR
ISR(ADC_vect)
#else
void adc_lld_loop(void)
#endif
{
#ifdef HAL_ADC_NISR
	if (!(ADCSRA & _BV(ADIF)))
		// conversion isn't done
		return;
	// clear Interrupt Flag
	ADCSRA |= _BV(ADIF);
#endif
	if (conversion_channel != 0xFF) {
		adc_lld_result[conversion_channel] = ADC;
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
	for (uint8_t i=0; i<ADC_LEN && !(conversion_mask & mask); i++) {
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

