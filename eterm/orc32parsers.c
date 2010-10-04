
/** ORC32 parsers
 * Parsers list:
 *   - '#' -- set position
 */

#include "eterm.h"
#include "hal/servo.h"

/// Debug print
#ifndef NDEBUG
#include <stdio.h>
#define debug(...) printf(__VA_ARGS__)
#else
#define debug(...)
#endif

typedef enum {
	SMP_GET_COMMAND,			///< get command ( '#', 'P', 'S', 'T', '\r', '\n' )
	SMP_ERROR,					///< skip all chars because command error, wait for '\r' or '\n'
	SMP_PARSE_NUMBER,			///< parse number after command
} state_cmd_t;

static bool servo_move_parser(char c, bool reinit) {
	static state_cmd_t state_cmd;
	static uint16_t _servo_target[SERVO_LEN];
	static uint16_t _servo_maxspeed[SERVO_LEN];
	static uint16_t _time2go;
	static uint16_t _servo=0;
	static uint16_t _num=0;
	static uint8_t _cmd=' ';

	if (reinit) {
		// Clear machine
		for (uint8_t i=0; i < SERVO_LEN; i++) {
			_servo_target[i] = 0;
			_servo_maxspeed[i] = 0;
		}
		state_cmd = SMP_GET_COMMAND;
		_time2go = 0;
		_cmd = ' ';
		_servo = 0;
		_num = 0;
	}

	c = toupper(c);

	switch (state_cmd) {
		case SMP_PARSE_NUMBER:
			if (c >= '0' && c <= '9') {
				_num = _num*10 + (c - '0');
				state_cmd = SMP_PARSE_NUMBER;
			} else {
				if (_cmd == '#') {
					if(_num < SERVO_LEN)
						_servo = _num;
				} else if (_cmd == 'P') {
					if (_num <= 2500 && _num >= 500) {
						_servo_target[_servo] = _num;
						debug("%% pos[%d]=%d\n", _servo, _num);
					}
				} else if (_cmd == 'S') {
					_servo_maxspeed[_servo] = _num;
					debug("%% spd[%d]=%d\n", _servo, _num);
				} else if (_cmd == 'T') {
					_time2go = _num;
					debug("%% time=%d\n", _num);
				}
				state_cmd = SMP_GET_COMMAND;
			}

		if (state_cmd != SMP_GET_COMMAND)
			break;

		case SMP_GET_COMMAND:
			switch(c) {
				case '#':
				case 'P':
				case 'S':
				case 'T':
					_cmd = c;
					_num = 0;
					state_cmd = SMP_PARSE_NUMBER;
					return false;

				case ' ':
					return false;

				case '\n':
					if (_cmd != ' ')
						servo_command(_time2go, _servo_target, _servo_maxspeed);
					return true;

				default:
					state_cmd = SMP_ERROR;
					return false;
			}
			break;

		default:
			state_cmd = SMP_ERROR;
			break;
		
	}

	if (c != '\n')
		return false;
	printf("ERR in # cmd\n");
	return true;
}


#define QSP_SELECT_CMD  100
#define QSP_Q_ERROR     101
#define QSP_QP_SN_ERROR 102

static bool query_status_parser(char c, bool reinit) {
	static uint8_t servo_num;

	if (reinit) {
		servo_num = QSP_SELECT_CMD;
		return false;
	}

	if (servo_num == QSP_SELECT_CMD) {
		if (c == '\n') {
			if (servo_is_done()) {
				printf(".");
			} else {
				printf("+");
			}
			return true;
		}
		if (c == 'P') {
			servo_num = 0;
			return false;
		}
		if (c != ' ')
			servo_num = QSP_Q_ERROR;
		return false;
	}

	if (servo_num == QSP_Q_ERROR) {
		if (c != '\n')
			return false;
		printf("ERR in Q cmd\n");
		return true;
	}
	
	if (servo_num == QSP_QP_SN_ERROR) {
		if (c != '\n')
			return false;
		printf("ERR in QP cmd servo #\n");
		return true;
	}

	if (servo_num < 100) {
		if (c >= '0' && c <= '9') {
			if (servo_num >= 10) {
				servo_num = QSP_QP_SN_ERROR;
				return false;
			}
			servo_num = servo_num * 10 + (c - '0');
			return false;
		}

		if (c == '\n') {
			char r = servo_get_position(servo_num)/10;
			putchar(r);
			return true;
		}

		if (c != ' ')
			servo_num = QSP_QP_SN_ERROR;
		return false;
	}

	return c == '\n';
}

static parser_t orc32parsers[] = {
	PARSER_INIT('#', "SSC-32 servo move", servo_move_parser),
	PARSER_INIT('Q', "SSC-32 query global status", query_status_parser)
};

void register_orc32(void) {
	for (uint8_t i=0; i < ARRAY_SIZE(orc32parsers); i++) {
		register_parser(orc32parsers + i);
	}
}

