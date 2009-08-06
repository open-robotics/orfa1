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
/** Serial command parser
 * @file parser.h
 *
 * @author Vladimir Ermakov <vooon341@gmail.com>
 */

#ifndef SERIAL_PARSER_H
#define SERIAL_PARSER_H

#include <stdint.h>
#include <stdbool.h>

#include "common.h"
#include "cbuf.h"

typedef enum {
	CMD_INIT,				///< clear cmd buffer
	GET_COMMAND,			///< get command ( 'S', 'R', 'W', '#', '\r', '\n' )
	COMMENT_OR_ERROR_EOL,	///< skip all chars, wait '\r' or '\n', change state to CMD_INIT
	PARSE_CLOCK,			///< parse config command ( C<f1><f2> )
	PARSE_LOCAL,			///< parse set local addr command  ( L<aa> )
	PARSE_I2C,				///< parse I2C commands ( S<aa><data>[S<aa><data>]P )
	PARSE_READ,				///< parse read register command ( R<aa><rr>[<ll>] )
	PARSE_WRITE,			///< parse write register command ( W<aa><rr><data> )
	WAIT_EOL,				///< skip all chars, wait '\r' or '\n', change state to EXEC_COMMAND
	EXEC_COMMAND,			///< start exec, change state to CMD_INIT
} state_cmd_t;

typedef enum {
	GET_ADDRESS,			///< get address (both register and i2c machine)
	GET_REGISTER,			///< get register (register machine)
	GET_LENGTH_OR_EOF,		///< get length (register machine)
	GET_DATA_OR_EOF,		///< get data (register machine)
	PARSE_I2C_READ,			///< parse i2c read, change state to WAIT_RESTART_OR_STOP
	PARSE_I2C_WRITE,		///< parse i2c write
	WAIT_RESTART_OR_STOP,	///< wait next i2c frame or stop
} state_i2creg_t;

/**
 * Parser state machine
 *
 * @param[in] co one char from serial stream
 * @param[out] *cmd_buf command buffer
 * @param[out] *error_code error
 * @returns true if command parsed
 */
bool parse_cmd(uint8_t co, cbf_t *cmd_buf, error_code_t *error_code);

#endif // SERIAL_PARSER_H

