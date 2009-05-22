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
/** ORFA with serial gate
 * @file main.c
 *
 * @author Vladimir Ermakov <vooon341@gmail.com>
 */

#include <stdint.h>
#include <stdio.h>

#include "serialgate/i2c.h"
#include "serialgate/common.h"
#include "serialgate/serialgate.h"
#include "core/driver.h"
#include "drivers/ports_driver.h"
#include "drivers/spi_driver.h"
#include "drivers/motor_driver.h"

#define BUF_LEN 65

static enum {
	GET_REGISTER,
	GET_DATA,
} state_i2c = GET_REGISTER;

static uint8_t register_addr = 0x00;
static uint8_t buf[BUF_LEN];
static uint8_t data_len = 0;
static uint8_t* read_ptr;
static bool is_restart = false;
static bool is_read = false;
static GATE_RESULT result = GR_OK;

/** Handle I2C Start event
 * @param[in] address device address
 * @param[in] flag Write/Read flag
 * @return true if success (always)
 */
bool cmd_start(uint8_t address, i2c_rdwr_t flag)
{
	debug("# > cmd_start(0x%02x, %i)\n", address, flag);

	if(is_restart && !is_read && data_len > 0)
	{
		result = gate_register_write(register_addr, buf+1, data_len);
	}

	state_i2c = GET_REGISTER;
	is_read = (address & 0x01) ? true : false;
	is_restart = true;
	data_len = 0;

	if(is_read)
	{
		data_len = BUF_LEN - 1;
		read_ptr = buf;
		result = gate_register_read(register_addr, buf, &data_len);
	}

	return true;
}

/** Handle I2C Stop event
 */
void cmd_stop(void)
{
	debug("# > cmd_stop()\n");

	is_restart = false;
	if(!is_read)
	{
		result = gate_register_write(register_addr, buf+1, data_len);
	}
}

/** Handle I2C master write (slave receiver)
 * @param[in] c writed byte
 * @return true if success
 */
bool cmd_txc(uint8_t c)
{
	switch(state_i2c)
	{
		case GET_REGISTER:
			register_addr = c;
			state_i2c = GET_DATA;
			break;

		case GET_DATA:
			{
			if((++data_len) < BUF_LEN)
				buf[data_len] = c;
			}
			break;
	};

	debug("# > cmd_txc(0x%02x)\n", c);
	return true;
}

/** Handle I2C master read (slave transmitter)
 * @param[out] *c byte for read
 * @param[in] ack ack/nack
 * @return true if success
 */
bool cmd_rxc(uint8_t *c, bool ack)
{
	debug("# > cmd_rxc(0x%02x, %i)\n", *c, ack);

	if(data_len > 0)
	{
		*c = *read_ptr++;
		--data_len;
	}

	if((data_len > 0) ^ ack)
	{
		return false; // Nack
	}
	return true;
}

int main(void)
{
	gate_init_introspection();
	init_ports_driver();
	init_spi_driver();
	init_motor_driver();

	i2c_set_handlers(&cmd_start, &cmd_stop, &cmd_txc, &cmd_rxc);

	serialgate_mainloop();

	return 0;
}
