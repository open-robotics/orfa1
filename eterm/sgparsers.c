
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

#define PROTOCOL_VERION_STRING "V1.2"
#define BUFF_SIZE 64
#define ARRAY_SIZE(arr)  (sizeof(arr)/sizeof(arr[0]))
#define is_i2c_read(addr) ((addr)&0x01)


static int8_t xtoi(char c) {
	c = toupper(c);
	if (c >= '0' && c <= '9') {
		return c - '0';
	} else if (c >= 'A' && c <= 'F') {
		return c - 'A' + 10;
	} else {
		return -1;
	}
}

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
		puts("%% i2c clear bus called");
		puts("X");
		return true;
	}
	return false;
}

bool local_parser(char c, bool reinit) {
	static uint8_t addr;
	if (reinit) {
		get_xbyte(c, &addr, true);
		return false;
	}

	if (get_xbyte(c, &addr, false)) {
		addr &= 0xfe;
		printf("%% set local 0x%02X\n", addr);
	}

	if (c == '\n') {
		printf("L%02X\n", addr);
		return true;
	}
	return false;
}

bool speed_parser(char c, bool reinit) {
	static uint8_t byte;
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
		printf("C%04X\n", speed);
		return true;
	}
	return false;
}

bool i2c_parser(char c, bool reinit) {
	static uint8_t byte;
	static uint8_t addr;
	static uint8_t buff[BUFF_SIZE];
	static uint8_t count;

	if (reinit) {
		count = 0;
		get_xbyte(c, &byte, true);
		return false;
	}

	c = toupper(c);

	if (c == 'P') {
		return false;
	}

	if (get_xbyte(c, &byte, false)) {
		if (count == 0) {
			addr = byte;
		}
		buff[count++] = byte;
		if (count > BUFF_SIZE-1) {
			count = BUFF_SIZE-1;
		}
	}

	if (c == '\n' || c == 'S') {
		if (is_i2c_read(addr)) {
			byte = buff[1];
			printf("i2c_read 0x%02X 0x%02X\n", addr, byte); // here read

			printf("SR");
			for (int i=0; i < count; i++)
				printf("%02X", buff[i]);
		} else {
			printf("i2c_write 0x%02X", addr);
			for (int i=1; i <= count-1; i++)
				printf(" %02X", buff[i]);
			printf("\n");
			printf("SW");
			for (int i=1; i <= count; i++)
				putchar('A');
		}
		
		// reset
		count = 0;
		get_xbyte(c, &byte, true);

		if (c == '\n') {
			printf("P\n");
			return true;
		}
	}

	return false;
}

parser_t sgparsers[] = {
	PARSER_INIT('%', "comment", comment_parser),
	PARSER_INIT('V', "protocol version", version_parser),
	PARSER_INIT('X', "clear i2c bus", clearbus_parser),
	PARSER_INIT('L', "set/get local address", local_parser),
	PARSER_INIT('C', "set/get i2c speed", speed_parser),
	PARSER_INIT('S', "i2c request", i2c_parser),
};

void register_serialgate(void) {
	parser_t *it = sgparsers;
	for (int i=0; i < ARRAY_SIZE(sgparsers); i++, it++) {
		register_parser(it);
	}
}

