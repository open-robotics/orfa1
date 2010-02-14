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
/** ADC low level driver header
 * @file adc_lld.h
 *
 * @author Andrey Demenev
 * @author Vladimir Ermakov
 */

#ifndef ADCLLD_H
#define ADCLLD_H

#include <stdint.h>
#include <stdbool.h>

#ifndef HAL_ADC_NISR
#define ADC_VOLATILE volatile
#else
#define ADC_VOLATILE
#endif

#define ADC_LEN 8

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
extern ADC_VOLATILE uint8_t adc_lld_config;

/** ADC result table
 */
extern ADC_VOLATILE uint16_t adc_lld_result[ADC_LEN];

#define adc_lld_is_10bit() \
	(adc_lld_config & 0x04)

#define adc_lld_is_8bit() \
	(!adc_lld_is_10bit())

/** Reconfigure ADC
 */
void adc_lld_reconfigure(uint8_t new_mask);

#if defined(HAL_ADC_NISR) || defined(__DOXYGEN__)
/** ADC periodic
 */
void adc_lld_loop(void);
#endif

#endif // ADCLLD_H

