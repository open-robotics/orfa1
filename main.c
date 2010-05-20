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

#include "core/i2cadapter.h"
#include "core/scheduler.h"

// -- virtual slave --

#define BUF_LEN 65

#define GET_REGISTER true
#define GET_DATA     false
static bool state_i2c = GET_REGISTER;

static uint8_t register_addr = 0x00;
static uint8_t buf[BUF_LEN];
static uint8_t data_len = 0;
static uint8_t* read_ptr;
static bool is_restart = false;
static bool is_read = false;
static bool prev_is_read = false;
static bool read_always = false;
static GATE_RESULT result = GR_OK;

/** Handle I2C Start event
 * @param[in] address device address
 * @param[in] flag Write/Read flag
 * @return true if success (always)
 */
bool i2c_start_handler(uint8_t flag)
{
	debug("# > i2c_start_handler(0x%02x, %i)\n", 0, flag);

	if (is_restart && !is_read && data_len > 0) {
		debug("# `-> gate_register_write(0x%02X, buf, %d)\n", register_addr, data_len);
		result = gate_register_write(register_addr, buf+1, data_len);
		data_len = 0;
	}

	state_i2c = GET_REGISTER;
	is_read = (flag == 0) ? false : true;
	is_restart = true;

	if ((is_read && !prev_is_read) || 
		(is_read && (!data_len || read_always)))
	{
		data_len = BUF_LEN - 1;
		read_ptr = buf;
		result = gate_register_read(register_addr, buf, &data_len);
		debug("# `-> gate_register_read(0x%02X, buf, %d)\n", register_addr, data_len);
	}

	prev_is_read = is_read;

	return true;
}

/** Handle I2C Stop event
 */
void i2c_stop_handler(void)
{
	debug("# > i2c_stop_handler()\n");

	is_restart = false;
	if (!is_read) {
		debug("# `-> gate_register_write(0x%02X, buf, %d)\n", register_addr, data_len);
		result = gate_register_write(register_addr, buf+1, data_len);
		data_len = 0;
	}
}

/** Handle I2C master write (slave receiver)
 * @param[in] c writed byte
 * @return true if success
 */
bool i2c_txc_handler(uint8_t c)
{
	if (state_i2c) {
		// Get register
		read_always = c & 0x80;
		register_addr = c & ~0x80;
		state_i2c = GET_DATA;
	} else {
		// Get data
		if ((++data_len) < BUF_LEN)
			buf[data_len] = c;
		else
			data_len = BUF_LEN - 1;
	}

	debug("# > i2c_txc_handler(0x%02x)\n", c);
	return true;
}

/** Handle I2C master read (slave transmitter)
 * @param[out] *c byte for read
 * @param[in] ack ack/nack
 * @return true if success
 */
bool i2c_rxc_handler(uint8_t *c, bool *ack)
{
	if (!data_len) {
		data_len = BUF_LEN - 1;
		read_ptr = buf;
		result = gate_register_read(register_addr, buf, &data_len);
		debug("# ,-> gate_register_read(0x%02X, buf, %d)\n", register_addr, data_len);
	}

	if (data_len > 0) {
		*c = *read_ptr++;
		--data_len;
	} else {
		*c = 0;
	}

	debug("# > i2c_rxc_handler(0x%02x, %i)\n", *c, *ack);
	return true;
}

// -- Main --

SYSTEM_INIT()
{
	// Set I2C
	i2c_set_handlers(i2c_start_handler, i2c_stop_handler,
			i2c_txc_handler, i2c_rxc_handler);
	// register supertask
	gate_supertask_register(gate_supertask);
	// register introspection driver
	gate_init_introspection();
}

/** Main
 */
int main(void)
{
	gate_init();
	asm volatile ("sei");
	gate_scheduler_loop();
	return 0;
}
