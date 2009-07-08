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
/** 4017 Servo driver
 * @file servo4017.c
 *
 * @author Andrey Demenev
 */


#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdbool.h>

#ifdef USE_EEPROM
#include <avr/eeprom.h>
#endif

#ifndef NDEBUG
#include <stdio.h>
#endif

#define US2CLOCK(us) (((uint32_t)(us) * (uint32_t)(F_CPU / 8000000.0 * 0x10000UL)) >> 16)

#define process_timer(OCRX, TCCRX, block, FOC_MASK) {	\
	OCRX += *(table_ptr[block]);						\
	if (table_ptr[block] == &(calc_ocr[block][8]) ) {	\
		table_ptr[block] = &(calc_ocr[block][0]);		\
		TCCRX |= FOC_MASK;								\
		TCCRX |= FOC_MASK;								\
	} else {											\
		table_ptr[block]++;								\
	}													\
	TCCRX |= FOC_MASK;									\
}

static uint8_t PROGMEM pin_map[32] = {
	7, 3, 2, 6, 5, 1, 0, 4,
	7, 3, 2, 6, 5, 1, 0, 4,
	4, 0, 1, 5, 6, 2, 3, 7,
	4, 0, 1, 5, 6, 2, 3, 7,
};

static uint16_t calc_ocr[4][9] = {
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

static uint16_t* table_ptr[4] = {
	calc_ocr[0],
	calc_ocr[1],
	calc_ocr[2],
	calc_ocr[3],
};


#ifdef USE_EEPROM
static uint8_t EEMEM ee_load_calc_ocr = false;
static uint16_t EEMEM ee_calc_ocr[4][9];

void s4017_save_positions(bool load_flag)
{
	#ifndef NDEBUG
	printf("# servo4017->save_positions(%i)\n", load_flag);
	#endif

	if (load_flag) {
		eeprom_write_block(calc_ocr, ee_calc_ocr, sizeof(calc_ocr));
	}
	eeprom_write_byte(&ee_load_calc_ocr, !load_flag);
}

void s4017_load_positions(void)
{
	uint8_t load_flag = !eeprom_read_byte(&ee_load_calc_ocr);
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


ISR(SIG_OUTPUT_COMPARE3A) {
	process_timer(OCR3A, TCCR3C, 2, (1 << FOC3A));
}

ISR(SIG_OUTPUT_COMPARE3B) {
	process_timer(OCR3B, TCCR3C, 0, (1 << FOC3B));
}

ISR(SIG_OUTPUT_COMPARE3C) {
	process_timer(OCR3C, TCCR3C, 3, (1 << FOC3C));
}

ISR(SIG_OUTPUT_COMPARE1C) {
	process_timer(OCR1C, TCCR1C, 1, (1 << FOC1C));
}

void s4017_set_position(uint8_t n, uint16_t pos)
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

void s4017_init(void)
{
	#ifdef USE_EEPROM
	s4017_load_positions();
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
}

