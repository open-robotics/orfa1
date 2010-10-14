/*
 *  ORFA -- Open Robotics Firmware Architecture
 *
 *  Copyright (c) 2010 Vladimir Ermakov
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
/** eTerm main module
 * @file eterm.h
 *
 * @author Vladimir Ermakov <vooon341@gmail.com>
 */

#ifndef ETERM_H
#define ETERM_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>

/** Parser init
 */
#define PARSER_INIT(_command, _help, _callback) \
	{ .command = _command, .help = _help, .callback = _callback }

#define ARRAY_SIZE(arr)  (sizeof(arr) / sizeof(arr[0]))

/** Parser callback
 * @param *c      current char
 * @param reinit  (re)initialize flag
 */
typedef bool (*parser_callback)(char c, bool reinit);

typedef struct parser_s parser_t;
struct parser_s {
	char command;
	char *help;
	parser_callback callback;
	struct parser_s *next;
};

/** Parser registration function
 * @param *parser  filled parser 
 */
void register_parser(parser_t *parser);

/** Parse the command
 */
bool parse_command(char c, bool reinit);

/** register '?' and 'h' commands
 */
void register_help(void);

#endif // !ETERM_H

