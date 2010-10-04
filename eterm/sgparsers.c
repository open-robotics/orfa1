
/** ORFA's serial gate compatible parsers
 * Parsers list:
 *   - % -- comment
 *   - V -- version
 *   - X -- clear i2c bus
 *   - L -- get/set local
 *   - C -- get/set i2c bus speed
 *   - S -- i2c request
 */

#include "eterm.h"
#include "lib/cbuf.h"
#include "lib/hex.h"
#include "hal/i2c.h"
#include <util/atomic.h>

#include <core/i2cadapter.h>

#define PROTOCOL_VERION_STRING "V1.2"
#define is_i2c_read(addr) ((addr)&0x01)

// -- common --

static uint8_t byte;
static uint8_t addr;
static uint8_t count;
static uint8_t read_count;
static cbf_t iobuff;

static bool get_xbyte(char c, uint8_t *ret, bool reinit) {
	static bool step;
	int8_t xi = xtoi(c);
	
	if (reinit) {
		step = false;
		return false;
	}

	if (c == '\n')
		return false;

	if (!step) {
		*ret = xi << 4;
		step = true;
		return false;
	} else {
		*ret |= xi;
		step = false;
		return true;
	}
}

static bool master_rx_handler(uint8_t c) {
	cbf_put(&iobuff, c);
	count++;
	return count < read_count;
}

static bool master_tx_handler(uint8_t *c, bool *ack) {
	bool ack_ = !cbf_isempty(&iobuff);
	*c = cbf_get(&iobuff);
	count++;
	return ack_;
}

// -- parsers --

bool comment_parser(char c, bool reinit) {
	return c == '\n';
}

bool version_parser(char c, bool reinit) {
	if (c == '\n') {
		puts(PROTOCOL_VERION_STRING);
		return true;
	}
	return false;
}

bool clearbus_parser(char c, bool reinit) {
	if (c == '\n') {
		i2c_clearbus();
		puts("X");
		return true;
	}
	return false;
}

bool local_parser(char c, bool reinit) {
	if (reinit) {
		get_xbyte(c, &byte, true);
		return false;
	}

	if (get_xbyte(c, &byte, false)) {
		byte >>= 1;
		i2c_set_local(byte);
	}

	if (c == '\n') {
		byte = i2c_get_local() << 1;
		putchar('L');
		putchar(itox(byte >> 4));
		putchar(itox(byte & 0xf));
		putchar('\n');
		return true;
	}
	return false;
}

bool speed_parser(char c, bool reinit) {
	static uint16_t speed;
	if (reinit) {
		speed = 0;
		get_xbyte(c, &byte, true);
		return false;
	}

	if (get_xbyte(c, &byte, false)) {
		speed <<= 8;
		speed |= byte;
	}

	if (c == '\n') {
		if (speed > 10 && speed < 600) {
			i2c_set_freq(speed);
		}

		speed = i2c_get_freq();
		putchar('C');
		putchar(itox((speed >> 12)));
		putchar(itox((speed >> 8) & 0xf));
		putchar(itox((speed >> 4) & 0xf));
		putchar(itox((speed) & 0xf));
		putchar('\n');
		return true;
	}
	return false;
}

bool i2c_parser(char c, bool reinit) {

	static uint8_t reg=0;
	static uint8_t buffer[65];

	if (reinit) {
		cbf_init(&iobuff);
		get_xbyte(c, &byte, true);
		return false;
	}

	c = toupper(c);

	if (c == 'P') {
		return false;
	}

	if (c == ' ') {
		return false;
	}

	if (get_xbyte(c, &byte, false)) {
		cbf_put(&iobuff, byte);
	}

	if (c == '\n' || c == 'S') {
		addr = cbf_get(&iobuff);
		if (is_i2c_read(addr)) {
			read_count = cbf_get(&iobuff);
			
			// flush
			count = 0;
			cbf_init(&iobuff);

			if (i2c_get_local() == addr >> 1) {
				//local request
				gate_register_read(reg, buffer, &read_count);
				for (uint8_t i=0; i<read_count; i++)
					cbf_put(&iobuff, buffer[i]);
			} else {
				//i2c bus real request
				i2c_request(addr >> 1);
			}

			putchar('S');
			putchar('R');
			while (!cbf_isempty(&iobuff)) {
				byte = cbf_get(&iobuff);
				putchar(itox(byte >> 4));
				putchar(itox(byte & 0xf));
			}
		} else {
			// flush
			count = 0;

			if (i2c_get_local() == addr >> 1) {
				//local request
				reg = cbf_get(&iobuff);
				while (!cbf_isempty(&iobuff))
					buffer[count++] = cbf_get(&iobuff);
				gate_register_write(reg, buffer, count);
			} else {
				//i2c bus real request
				i2c_start_transmission(addr >> 1);
			}
			
			putchar('S');
			putchar('W');
			for (int i=0; i < count; i++)
				putchar('A');
		}
		
		// reset
		get_xbyte(c, &byte, true);

		if (c == '\n') {
			puts("P");
			return true;
		}
	}

	return false;
}

// -- table --

parser_t sgparsers[] = {
	PARSER_INIT('%', "comment", comment_parser),
	PARSER_INIT('V', "protocol version", version_parser),
	PARSER_INIT('X', "clear i2c bus", clearbus_parser),
	PARSER_INIT('L', "set/get local address", local_parser),
	PARSER_INIT('C', "set/get i2c speed", speed_parser),
	PARSER_INIT('S', "i2c request", i2c_parser),
};

void register_serialgate(void) {
	i2c_set_master_handlers(master_rx_handler, master_tx_handler);
	for (int i=0; i < ARRAY_SIZE(sgparsers); i++) {
		register_parser(sgparsers + i);
	}
}

