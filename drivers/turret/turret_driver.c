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

#include "core/common.h"
#include "core/driver.h"
#include "core/scheduler.h"
#include <avr/io.h>
#include <stdint.h>
#include <stdbool.h>

static GATE_RESULT driver_read(uint8_t reg, uint8_t* data, uint8_t* data_len);
static GATE_RESULT driver_write(uint8_t reg, uint8_t* data, uint8_t data_len);
static void turret_task(void);

static GATE_DRIVER driver = {
	.uid = 0xff02, // turret id
	.major_version = 1,
	.minor_version = 0,
	.read = driver_read,
	.write = driver_write,
	.num_registers = 1,
};

static GATE_TASK task = {
	.task = turret_task,
};

static enum {
	T_STOP=0  //!< stop turret
	T_LEFT,   //!< turn left
	T_RIGHT,  //!< turn right
	T_CENTER, //!< go to center
} operation = T_STOP;

static GATE_RESULT driver_read(uint8_t reg, uint8_t* data, uint8_t* data_len)
{
	if (!*data_len) {
		return GR_OK;
	}
	
	*data_len = 1;
	*data = (uint8_t) operation;

	return GR_OK;
}

static GATE_RESULT driver_write(uint8_t reg, uint8_t* data, uint8_t data_len)
{	
	if (data_len == 1) {
		operation = *data;
		if (operation > 3)
			operation = T_STOP;
	}
	// else error...

	return GR_OK;
}

static void turret_task(void)
{
	debug("# turret task\n");
}

GATE_RESULT init_turret_driver(void)
{
	//gate_task_register(&task);
	(void)task;

	return gate_driver_register(&driver);
}

