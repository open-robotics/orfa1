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

#include <avr/io.h>
#include "core/common.h"
#include "core/driver.h"
#include <stdint.h>

// User UIDs 0xff00 — 0xffff
#define EXAMPLE_UID 0xFF55

static GATE_RESULT
example_driver_read(uint8_t reg, uint8_t* data, uint8_t* data_len);
static GATE_RESULT
example_driver_write(uint8_t reg, uint8_t* data, uint8_t data_len);

#define REG1 0x0
// #define REG2 0x01
// #define REG2 0x02

static GATE_DRIVER example_driver = {
	.uid = EXAMPLE_UID,
	.major_version = 1,
	.minor_version = 0,
	.read = example_driver_read,
	.write = example_driver_write,
	.num_registers = 1,
};

static GATE_RESULT
example_driver_read(uint8_t reg, uint8_t* data, uint8_t* data_len)
{
	switch (reg) {
		// ....
	};

	return GR_OK;
}

static GATE_RESULT
example_driver_write(uint8_t reg, uint8_t* data, uint8_t data_len)
{
	switch (reg)
	{
		// ....
	}
	
	return GR_OK;
}

GATE_RESULT init_example_driver(void)
{
	// ...
	// ...
	// ...
	return gate_driver_register(&example_driver);
}

