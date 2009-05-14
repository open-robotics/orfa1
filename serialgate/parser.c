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

#include "parser.h"

/**
 * Convert hex to int8 (0 — 15)
 * @param[in] c 0 — 9, A — F
 * @return -1 if fail
 */
static inline int8_t xtoi(uint8_t c)
{
	if((c >= '0')&&(c <= '9'))
	{
		return c - '0';
	}
	else if((c >= 'A')&&(c <= 'F'))
	{
		return c - 'A' + 10;
	}
	else
	{
		return -1;
	}
}

/**
 * Get one byte state machine
 * @param[in] c char
 * @param[out] *data received byte
 * @param[out] *error_code error reporting
 * @return true when receive complete
 */
static inline bool get_cmd_byte(uint8_t c, uint8_t *data, error_code_t *error_code)
{
	static uint8_t cout = 1;
	int8_t tmp = xtoi(c);
	if(tmp >= 0 && cout == 1)
	{
		// get first nibble
		*data = tmp << 4;
		cout = 2;
	}
	else if(tmp >= 0 && cout == 2)
	{
		// get second nibble
		*data |= tmp;
		cout = 1;
		return true;
	}
	else if(c == ' ' && cout == 1)
	{
		// skip spaces
	}
	else
	{
		cout = 1;
		*error_code = INVALID_XDIGIT;
	}

	return false;
}


bool parse_cmd(uint8_t co, cbf_t *cmd_buf, error_code_t *error_code)
{
	// temp
	int8_t i;
	uint8_t c=toupper(co);

	// static per iteration data
	static uint8_t cout=1;
	static uint8_t data=0;
	static uint8_t addr=0;
	static uint8_t reg_len=1;
	static error_code_t error_c=NO_ERROR;

	// machine states
	static state_cmd_t state_cmd=CMD_INIT;
	static state_i2creg_t state_i2creg=GET_ADDRESS;

	if(state_cmd == CMD_INIT)
	{
		// Clear machine
		addr = 0;
		cout = 1;
		data = 0;
		reg_len = 1;
		cbf_init(cmd_buf);
		state_cmd = GET_COMMAND;
		state_i2creg = GET_ADDRESS;
		error_c = *error_code;
	}

	// ---------------------------------------------------------------

	switch(state_cmd)
	{
		case GET_COMMAND:
			switch(c)
			{
				case 'S':
					// is i2c?
					state_cmd = PARSE_I2C;
					cbf_put(cmd_buf, 'S');
					break;

				case 'R':
					// is read register?
					state_cmd = PARSE_READ;
					cbf_put(cmd_buf, 'S');
					break;

				case 'W':
					// is write register?
					state_cmd = PARSE_WRITE;
					cbf_put(cmd_buf, 'S');
					break;

				case 'C':
					cbf_put(cmd_buf, c);
					state_cmd = PARSE_CONFIG;
					break;

				case 'L':
					cbf_put(cmd_buf, c);
					state_cmd = PARSE_LOCAL;
					break;

				case 'V':
				case 'X':
					cbf_put(cmd_buf, c);
					state_cmd = WAIT_EOL;
					break;

				case '\r':
				case '\n':
					break;

				case '#':
					// is comment?
					state_cmd = COMMENT_OR_ERROR_EOL;
					break;

				default:
					error_c = INVALID_COMMAND;
					break;
			}
			break;

		case COMMENT_OR_ERROR_EOL:
			// skip line
			if(c == '\r' || c == '\n')
			{
				*error_code = error_c;
				state_cmd = CMD_INIT;
			}
			break;

		case PARSE_CONFIG:
			if(get_cmd_byte(c, &data, &error_c))
			{
				cbf_put(cmd_buf, data);
				if(cout++ > 1)
				{
					state_cmd = WAIT_EOL;
				}
			}
			break;

		case PARSE_LOCAL:
			if(get_cmd_byte(c, &data, &error_c))
			{
				cbf_put(cmd_buf, data);
				state_cmd = WAIT_EOL;
			}
			break;

		case PARSE_I2C:
			switch(state_i2creg)
			{
				case GET_ADDRESS:
					if(get_cmd_byte(c, &data, &error_c))
					{
						cbf_put(cmd_buf, data);
						state_i2creg = (data & 0x01) ? PARSE_I2C_READ : PARSE_I2C_WRITE;
					}
					break;

				case PARSE_I2C_READ:
					if(get_cmd_byte(c, &data, &error_c))
					{
						cbf_put(cmd_buf, data);
						state_i2creg = WAIT_RESTART_OR_STOP;
					}
					break;

				case WAIT_RESTART_OR_STOP:
					switch(c)
					{
						case 'S':
							// reSTART?
							cbf_put(cmd_buf, c);
							state_i2creg = GET_ADDRESS;
							break;

						case 'P':
							// STOP?
							cbf_put(cmd_buf, c);
							state_cmd = WAIT_EOL;
							break;

						default:
							break;
					}
					break;

				case PARSE_I2C_WRITE:
					i = xtoi(c);
					if(c == '\\' && cout == 1)
					{
						// masked byte?
						cout = '\\';
					}
					else if(cout == '\\')
					{
						// get masked byte
						cbf_put(cmd_buf, 1);
						cbf_put(cmd_buf, co);
						cout = 1;
					}
					else if(i >= 0 && cout == 1)
					{
						// get first nibble
						data = i << 4;
						cout = 2;
					}
					else if(i >= 0 && cout == 2)
					{
						// get second nibble
						data |= i;
						cbf_put(cmd_buf, 1);
						cbf_put(cmd_buf, data);
						cout = 1;
					}
					else if(c == 'S' && cout == 1)
					{
						// reSTART?
						cbf_put(cmd_buf, c);
						state_i2creg = GET_ADDRESS;
					}
					else if(c == 'P' && cout == 1)
					{
						// STOP?
						cbf_put(cmd_buf, c);
						state_cmd = WAIT_EOL;
					}
					else if(c == ' ' && cout == 1)
					{
						// skip spaces
					}
					else
					{
						error_c = INVALID_DATA;
					}
					break;

				} // parse_i2c state machine
				break;

		case PARSE_READ:
			switch(state_i2creg)
			{
				case GET_ADDRESS:
					if(get_cmd_byte(c, &data, &error_c))
					{
						addr = data & (~0x01);
						cbf_put(cmd_buf, addr);
						state_i2creg = GET_REGISTER;
					}
					break;

				case GET_REGISTER:
					if(get_cmd_byte(c, &data, &error_c))
					{
						cbf_put(cmd_buf, 1);
						cbf_put(cmd_buf, data);
						cbf_put(cmd_buf, 'S');
						cbf_put(cmd_buf, addr|0x01);
						state_i2creg = GET_LENGTH_OR_EOF;
					}
					break;

				case GET_LENGTH_OR_EOF:
					i = xtoi(c);
					if(i >= 0 && cout == 1)
					{
						// get first nibble
						reg_len = i << 4;
						cout = 2;
					}
					else if(i >= 0 && cout == 2)
					{
						// get second nibble
						reg_len |= i;
						cout = 1;
					}
					else if(c == ' ' && cout != 2)
					{
						// skip spaces
					}
					else if((c == '\r' || c == '\n') && cout != 2)
					{
						cbf_put(cmd_buf, reg_len);
						cbf_put(cmd_buf, 'P');
						cbf_put(cmd_buf, '\n');
						state_cmd = EXEC_COMMAND;
					}
					break;

				default:
					*error_code = INTERNAL_ERROR;
					break;
			}
			break;

		case PARSE_WRITE:
			switch(state_i2creg)
			{
				case GET_ADDRESS:
					if(get_cmd_byte(c, &data, &error_c))
					{
						addr = data & (~0x01);
						cbf_put(cmd_buf, addr);
						state_i2creg = GET_REGISTER;
					}
					break;

				case GET_REGISTER:
					if(get_cmd_byte(c, &data, &error_c))
					{
						cbf_put(cmd_buf, 1);
						cbf_put(cmd_buf, data);
						state_i2creg = GET_DATA_OR_EOF;
					}
					break;

				case GET_DATA_OR_EOF:
					i = xtoi(c);
					if(c == '\\' && cout == 1)
					{
						// masked byte?
						cout = '\\';
					}
					else if(cout == '\\')
					{
						// get masked byte
						cbf_put(cmd_buf, 1);
						cbf_put(cmd_buf, co);
						cout = 1;
					}
					else if(i >= 0 && cout == 1)
					{
						// get first nibble
						data = i << 4;
						cout = 2;
					}
					else if(i >= 0 && cout == 2)
					{
						// get second nibble
						data |= i;
						cbf_put(cmd_buf, 1);
						cbf_put(cmd_buf, data);
						cout = 1;
					}
					else if((c == '\r' || c == '\n') && cout == 1)
					{
						cbf_put(cmd_buf, 'P');
						cbf_put(cmd_buf, '\n');
						state_cmd = EXEC_COMMAND;
					}
					else if(c == ' ' && cout == 1)
					{
						// skip spaces
					}
					else
					{
						error_c = INVALID_DATA;
					}
					break;

				default:
					*error_code = INTERNAL_ERROR;
					break;
			}
			break;

		case WAIT_EOL:
			if(c == '\r' || c == '\n')
			{
				// EOL? (exec)
				cbf_put(cmd_buf, '\n');
				state_cmd = EXEC_COMMAND;
			}
			break;

		default:
			*error_code = INTERNAL_ERROR;
			break;
	}

	// ---------------------------------------------------------------

	if(error_c != NO_ERROR)
	{
		state_cmd = COMMENT_OR_ERROR_EOL;
	}

	if(*error_code != NO_ERROR)
	{
		state_cmd = CMD_INIT;
	}

	if(state_cmd == EXEC_COMMAND)
	{
		state_cmd = CMD_INIT;
		return true;
	}

	return false;
}

