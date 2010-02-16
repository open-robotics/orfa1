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
/** RoboMD2 Motor I2C adapter
 * @file motor_i2c.c
 * @author Vladimir Ermakov
 *
 * @todo use ATOMIC_BLOCK()
 *
 * @note New in version 1.1:
 *       now you can set all registers in one request
 */

/**
 * @ingroup MotorAdapter
 * @{
 */

#include "core/i2cadapter.h"

#include "hal/motor.h"

static GATE_RESULT
motor_i2cadapter_read(uint8_t reg, uint8_t* data, uint8_t* data_len);
static GATE_RESULT
motor_i2cadapter_write(uint8_t reg, uint8_t* data, uint8_t data_len);

/// PWM register channel 0
#define PWM_REG0   0x00
/// PWM register channel 1
#define PWM_REG1   0x01
/// Direction register channel 0
#define DIR_REG0   0x02
/// Direction register channel 1
#define DIR_REG1   0x03

static GATE_I2CADAPTER motor_i2cadapter = {
	.uid = 0x0060, // motor id
	.major_version = 1,
	.minor_version = 1,
	.read = motor_i2cadapter_read,
	.write = motor_i2cadapter_write,
	.num_registers = 4,
};

#define GET_PWM(ch) \
	case PWM_REG##ch: *data = motor_get_pwm(ch); break

#define GET_DIR(ch) \
	case DIR_REG##ch: *data = motor_get_direction(ch); break

static GATE_RESULT motor_i2cadapter_read(uint8_t reg, uint8_t* data, uint8_t* data_len)
{
	if (!*data_len) {
		return GR_OK;
	}

	*data_len = 1;
	switch (reg)
	{
		GET_PWM(0);
		GET_PWM(1);
		GET_DIR(0);
		GET_DIR(1);

		default:
			return GR_INVALID_REGISTER;
	}
	
	return GR_OK;
}

#undef GET_PWM
#undef GET_DIR

#define SET_PWM(ch) \
	case PWM_REG##ch: motor_set_pwm(ch, *data); break

#define SET_DIR(ch) \
	case DIR_REG##ch: motor_set_direction(ch, *data); break

static GATE_RESULT motor_i2cadapter_write(uint8_t reg, uint8_t* data, uint8_t data_len)
{
	if (!data_len)
		return GR_INVALID_ARG;

	if (data_len == 4) {
		motor_set_pwm(0, data[0]);
		motor_set_pwm(1, data[1]);
		motor_set_direction(0, data[2]);
		motor_set_direction(1, data[3]);
		return GR_OK;
	}

	switch (reg)
	{
		SET_PWM(0);
		SET_PWM(1);
		SET_DIR(0);
		SET_DIR(1);

		default:
			return GR_INVALID_REGISTER;
	}
	
	return GR_OK;
}

#undef SET_PWM
#undef SET_DIR

// Autoinit
I2C_MODULE_INIT(motor_adapter)
{
	motor_init();
	gate_i2cadapter_register(&motor_i2cadapter);
}

