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
/** Servo I2C adapter
 * @file servo_i2c.c
 *
 * @author Andrey Demenev
 * @author Vladimir Ermakov
 */

#include "servo_i2c.h"

static GATE_RESULT
servo_i2cadapter_read(uint8_t reg, uint8_t* data, uint8_t* data_len);
static GATE_RESULT
servo_i2cadapter_write(uint8_t reg, uint8_t* data, uint8_t data_len);

static GATE_I2CADAPTER servo_i2cadapter = {
	.uid = SERVO_UID,
	.major_version = SERVO_MAJOR,
	.minor_version = SERVO_MINOR,
	.read = servo_i2cadapter_read,
	.write = servo_i2cadapter_write,
	.num_registers = 2,
};

static GATE_RESULT
servo_i2cadapter_read(uint8_t reg, uint8_t* data, uint8_t* data_len)
{
	*data_len = 0;
	return GR_OK;
}

static GATE_RESULT
servo_i2cadapter_write(uint8_t reg, uint8_t* data, uint8_t data_len)
{
	debug("i2c-servo-adapter\n");
	
	if (reg > 1) {
		return GR_NO_ACCESS;
	}

	debug("lev-1\n");

	if (!reg) {
		return GR_OK;
	}

	debug("lev-2\n");

	if (data_len < 3) {
		return GR_INVALID_DATA;
	}

	debug("lev-3\n");

	uint16_t _servo_target[32];
	uint16_t _servo_maxspeed[32];
	uint16_t _max_time=0;
	for(int i=0; i<32; i++){
		_servo_target[i]=0;
		_servo_maxspeed[i]=0;
	};
	while (data_len) {
		uint16_t val=(data[1]<<8)|data[2];
		uint8_t id=data[0];
		if(id<128){	_servo_target[id]=val; } else
		if(id<255){ _servo_maxspeed[id-128]=val; }else
		if(id==255){ _max_time=val; };
		data += 3;
		data_len -= 3;
		if (data_len>0) if (data_len < 3 || data_len > 252) {
			return GR_INVALID_DATA;
		};
	};

	debug("lev-4\n");

	servo_command(_max_time,_servo_target,_servo_maxspeed);

	debug("lev-5\n");

	return GR_OK;
}

I2C_MODULE_INIT(servo_adapter)
{
	servo_init();
	gate_i2cadapter_register(&servo_i2cadapter);
}

