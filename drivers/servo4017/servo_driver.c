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

#ifndef OR_AVR_M64_S
#error servo4017: unsupported platform
#endif

#include <avr/io.h>
#include "core/common.h"
#include "core/driver.h"
#include <stdint.h>
#include <stdbool.h>
#include <avr/pgmspace.h>

#ifdef USE_EEPROM
#include <avr/eeprom.h>
#endif

#ifndef NDEBUG
#include <stdio.h>
#endif

#define US2CLOCK(us) (((uint32_t)(us) * (uint32_t)(F_CPU / 8000000.0 * 0x10000UL)) >> 16)

static GATE_RESULT driver_read(uint8_t reg, uint8_t* data, uint8_t* data_len);
static GATE_RESULT driver_write(uint8_t reg, uint8_t* data, uint8_t data_len);

static GATE_DRIVER driver = {
	.uid = 0x30,
	.major_version = 1,
	.minor_version = 0,
	.read = driver_read,
	.write = driver_write,
	.num_registers = 2,
};


static uint8_t PROGMEM pin_map[32] = {
	7, 3, 2, 6, 5, 1, 0, 4,
	7, 3, 2, 6, 5, 1, 0, 4,
	4, 0, 1, 5, 6, 2, 3, 7,
	4, 0, 1, 5, 6, 2, 3, 7,
};

uint16_t calc_ocr[4][9] = {
	{
		US2CLOCK(1500), US2CLOCK(1500), US2CLOCK(1500), US2CLOCK(1500),
		US2CLOCK(1500), US2CLOCK(1500), US2CLOCK(1500), US2CLOCK(1500),
		US2CLOCK(20500 - 8 * 1500),
	},
	{
		US2CLOCK(1500), US2CLOCK(1500), US2CLOCK(1500), US2CLOCK(1500),
		US2CLOCK(1500), US2CLOCK(1500), US2CLOCK(1500), US2CLOCK(1500),
		US2CLOCK(20500 - 8 * 1500),
	},
	{
		US2CLOCK(1500), US2CLOCK(1500), US2CLOCK(1500), US2CLOCK(1500),
		US2CLOCK(1500), US2CLOCK(1500), US2CLOCK(1500), US2CLOCK(1500),
		US2CLOCK(20500 - 8 * 1500),
	},
	{
		US2CLOCK(1500), US2CLOCK(1500), US2CLOCK(1500), US2CLOCK(1500),
		US2CLOCK(1500), US2CLOCK(1500), US2CLOCK(1500), US2CLOCK(1500),
		US2CLOCK(20500 - 8 * 1500),
	},
};

uint16_t* table_ptr[4] = {
	calc_ocr[0],
	calc_ocr[1],
	calc_ocr[2],
	calc_ocr[3],
};

static void set_position(uint8_t n, uint16_t pos)
{
	if (n > 31) return;
	if (pos < 500) pos = 500;
	else if (pos > 2500) pos = 2500;
	uint8_t idx = pgm_read_byte(pin_map+n);
	uint8_t block = n >> 3;
	pos = US2CLOCK(pos);
	calc_ocr[block][8] += calc_ocr[block][idx];
	calc_ocr[block][8] -= pos;
	calc_ocr[block][idx] = pos;
}

#ifdef USE_EEPROM
uint8_t EEMEM ee_load_calc_ocr = false;
uint16_t EEMEM ee_calc_ocr[4][9];

static void save_positions(bool load_flag)
{
#ifndef NDEBUG
	printf("# servo4017->save_positions(%i)\n", load_flag);
#endif

	if (load_flag) {
		eeprom_write_block(calc_ocr, ee_calc_ocr, sizeof(calc_ocr));
	}
	eeprom_write_byte(&ee_load_calc_ocr, load_flag);
}

static void load_positions(void)
{
	uint8_t load_flag = eeprom_read_byte(&ee_load_calc_ocr);
#ifndef NDEBUG
	printf("# servo4017->load_positions()\n# :: load_flag = %i\n", load_flag);
#endif

	if (load_flag) {
		eeprom_read_block(calc_ocr, ee_calc_ocr, sizeof(calc_ocr));

#ifndef NDEBUG
		printf("# :: Loaded:\n");
		for (uint8_t i=0; i<4; i++) {
			printf("# [%i]:", i);
			for (uint8_t j=0; j<9; j++) {
				printf(" 0x%04X", calc_ocr[i][j]);
			}
			printf("\n");
		}
#endif
	}
}
#endif

static GATE_RESULT driver_read(uint8_t reg, uint8_t* data, uint8_t* data_len)
{
	*data_len = 0;
	return GR_OK;
}

static GATE_RESULT driver_write(uint8_t reg, uint8_t* data, uint8_t data_len)
{
	if (reg > 1) {
		return GR_NO_ACCESS;
	}
	if (!reg) {
#ifdef USE_EEPROM
		save_positions(*data);
#endif
		return GR_OK;
	}
	if (data_len < 3) {
		return GR_INVALID_DATA;
	}
	while (data_len) {
		uint16_t pos = data[2] + (data[1] << 8);
		set_position(*data, pos);
		data += 3;
		data_len -= 3;
		if (data_len < 3 || data_len > 252) {
			return GR_INVALID_DATA;
		}
	}
	return GR_OK;
}

GATE_RESULT init_servo_driver(void)
{
#ifdef USE_EEPROM
	load_positions();
#endif

	DDRE = _BV(2) | _BV(3) | _BV(4) | _BV(5);
	DDRB = _BV(7);

	PORTE |= _BV(2);
	
	OCR3A = 2000;
	OCR3B = 2000;
	OCR3C = 2000;
	OCR1C = 2000;

	TCCR3A = _BV(COM3A0) | _BV(COM3B0) | _BV(COM3C0);
	TCCR3B = _BV(CS31);

	TCCR1A = _BV(COM1C0);
	TCCR1B = _BV(CS11);

	ETIMSK |= _BV(OCIE1C) | _BV(OCIE3A) | _BV(OCIE3B) | _BV(OCIE3C);

	PORTE &= ~_BV(2);

	return gate_driver_register(&driver);
}

