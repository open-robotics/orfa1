
#include <stdint.h>
#include <stdio.h>

#include "serialgate/i2c.h"
#include "serialgate/common.h"
#include "serialgate/serialgate.h"
#include "registers/driver.h"
#include "registers/ports_driver.h"
#include "registers/spi_driver.h"
#include "registers/motor_driver.h"

#define BUF_LEN 65

static enum {
	GET_REGISTER,
	GET_DATA,
} state_i2c = GET_REGISTER;

static uint8_t register_addr = 0x00;
static uint8_t buf[BUF_LEN];
static uint8_t data_len = 0;
static bool is_restart = false;
static bool is_read = false;
static GATE_RESULT result = GR_OK;

/*! Handle I2C Start event
 * \param[in] address device address
 * \param[in] flag Write/Read flag
 * \return true if success (always)
 */
bool cmd_start(uint8_t address, i2c_rdwr_t flag)
{
	debug("# > cmd_start(0x%02x, %i)\n", address, flag);

	if(is_restart && !is_read && data_len > 0 && register_addr != 0x00)
	{
		result = gate_register_write(register_addr, buf+1, data_len);
	}

	state_i2c = GET_REGISTER;
	is_read = (address & 0x01) ? true : false;
	is_restart = true;
	data_len = 0;

	if(is_read && register_addr != 0x00)
	{
		data_len = BUF_LEN - 1;
		result = gate_register_read(register_addr, buf, &data_len);
	}

	return true;
}

/*! Handle I2C Stop event
 */
void cmd_stop(void)
{
	debug("# > cmd_stop()\n");

	is_restart = false;
	if(!is_read && register_addr != 0x00)
	{
		result = gate_register_write(register_addr, buf+1, data_len);
	}
}

/*! Handle I2C master write (slave receiver)
 * \param[in] c writed byte
 * \return true if success
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

/*! Handle I2C master read (slave transmitter)
 * \param[out] *c byte for read
 * \param[in] ack ack/nack
 * \return true if success
 */
bool cmd_rxc(uint8_t *c, bool ack)
{
	debug("# > cmd_rxc(0x%02x, %i)\n", *c, ack);

	if(register_addr == 0x00)
	{
		// error register
		*c = result;
		result = GR_OK; // clear
		return true;
	}

	if(data_len > 0)
	{
		*c = buf[data_len-1];
		--data_len;
	}
	if((data_len > 0) ^ ack)
	{
		return false; // Nack
	}

	return true;
}

#ifndef NDEBUG
#include "registers/driver.h"
static GATE_RESULT driver_read(uint8_t reg, uint8_t* data, uint8_t* data_len)
{
	debug("# driver_read(0x%02X, *data, 0x%02X)\n", reg, *data_len);
	data[0] = 0x00;
	data[1] = 0x01;
	data[2] = 0x02;
	data[3] = 0x03;
	data[4] = 0x04;
	*data_len = 5;
}

static GATE_RESULT driver_write(uint8_t reg, uint8_t* data, uint8_t data_len)
{
	debug("# driver_write(0x%02X, *data, 0x%02X)\n# > data->", reg, data_len);
	for(uint8_t i=0; i < data_len; i++)
	{
		debug(" 0x%02X", data[i]);
	}
	debug("\n");
}

static GATE_RESULT driver_init(void)
{
	debug("# driver_init()\n");
}

static uint8_t registers[] = {0x01, 0x02};
static GATE_DRIVER driver = {
	.read = driver_read,
	.write = driver_write,
	.init = driver_init,
	.registers = registers,
	.num_registers = NUM_ELEMENTS(registers),
};
#endif // NDEBUG

int main()
{
	init_ports_driver();
	init_spi_driver();
	init_motor_driver();

	#ifndef NDEBUG
	gate_driver_register(&driver);
	#endif

	i2c_set_handlers(&cmd_start, &cmd_stop, &cmd_txc, &cmd_rxc);

	serialgate_mainloop();

	return 0;
}
