/*
 *  ORFA -- Open Robotics Firmware Architecture
 *
 *  Copyright (c) 2009 Vladimir Ermakov, Andrey Demenev, Anton Botov
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

#include <stdio.h>
#include <ctype.h>
#include "parser.h"
#include "hal/servo.h"

bool parse_cmd(uint8_t co, error_code_t *error_code)
{
	// temp
	uint8_t c=toupper(co);
	c = (c == '\r')? '\n' : c;

	// static per iteration data
	static error_code_t error_c=NO_ERROR;
	static uint16_t _servo_target[SERVO_LEN];
	static uint16_t _servo_maxspeed[SERVO_LEN];
	static uint16_t _time2go;
	static uint16_t _servo=0;
	static uint16_t _num=0;
	static uint8_t _cmd=' ';

	// machine states
	static state_cmd_t state_cmd=CMD_INIT;

	if (state_cmd == CMD_INIT) {
		// Clear machine
		for (uint8_t i=0; i<SERVO_LEN; i++) {
			_servo_target[i] = 0;
			_servo_maxspeed[i] = 0;
		};
		_time2go = 0;
		_cmd = ' ';
		state_cmd = GET_COMMAND;
		error_c = *error_code;
	}

	debug("# c=%c, _cmd=%c\n", c, _cmd);
	//putchar(c);

	switch(state_cmd) {
		case PARSE_NUMBER:
			if (c >= '0' && c <= '9') {
				_num = _num*10 + (c-'0');
				state_cmd = PARSE_NUMBER;
			} else {
				if (_cmd == '#' || _cmd == 'G')
					if(_num < SERVO_LEN)
						_servo = _num;
				if (_cmd == 'P')
					if (_num <= 2500 && _num >= 500) {
					_servo_target[_servo] = _num;
					debug("# pos[%d]=%d\n", _servo, _num);
				}
				if (_cmd == 'S') {
					_servo_maxspeed[_servo] = _num;
					debug("# spd[%d]=%d\n", _servo, _num);
				}
				if (_cmd == 'T') {
					_time2go = _num;
					debug("# time=%d\n", _num);
				}
				state_cmd = GET_COMMAND;
			}
			if (state_cmd != GET_COMMAND)
				break;

		case GET_COMMAND:
			switch(c) {
				case '#':
				case 'P':
				case 'S':
				case 'T':
					if (_cmd == 'Q' && c == 'P') {
						_cmd = 'G';
						_num = 0;
						state_cmd = PARSE_NUMBER;
					} else {
						_cmd = c;
						_num = 0;
						state_cmd = PARSE_NUMBER;
					}
					break;

				case 'Q':
					_cmd = c;
					state_cmd = GET_COMMAND;
					break;

				case 'V':
					_cmd = c;
					printf("ORC-32 v0.9 (open-robotics.ru)\n");
					state_cmd = COMMENT_OR_ERROR_EOL;

				case ' ':
					break;

				case '\n':
					if (_cmd == ' ')
						break;

					if (_cmd == 'G') {
						uint16_t p = servo_get_position(_servo);
						uint8_t pc = p/10;
						putchar(pc);
						break;
					}

					if (_cmd == 'Q') {
						if (servo_is_done()) {
							putchar('.');
						} else {
							putchar('+');
						}
						putchar('\n');
						break;
					}

					servo_command(_time2go, _servo_target, _servo_maxspeed);
					state_cmd = CMD_INIT;
					break;

				default:
					error_c = INVALID_COMMAND;
					break;
			}
			break;

		case COMMENT_OR_ERROR_EOL:
			// skip line
			if (c == '\n') {
				*error_code = error_c;
				state_cmd = CMD_INIT;
			}
			break;

		default:
			*error_code = INTERNAL_ERROR;
			break;
	}

	// ---------------------------------------------------------------

	if (error_c != NO_ERROR) {
		state_cmd = COMMENT_OR_ERROR_EOL;
	}

	if (*error_code != NO_ERROR) {
		state_cmd = CMD_INIT;
	}

	return false;
}

