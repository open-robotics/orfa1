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
 * @file eterm.c
 *
 * @author Vladimir Ermakov <vooon341@gmail.com>
 */

#include "eterm.h"

parser_t *rootparser;

static parser_t *find_parser(char command) {
	parser_t *it=rootparser;
	command = toupper(command);
	while (it) {
		if (it->command == command) {
			return it;
		}
		it = it->next;
	}
	return NULL;
}

void register_parser(parser_t *parser) {
	parser_t *alredy_added = find_parser(parser->command);
	if (alredy_added) {
		//perror("parser alredy added");
		return;
	}
	parser->next = rootparser;
	rootparser = parser;
}

bool parse_command(char c, bool reinit) {
	static parser_t *currparser;
	bool res;

	if (reinit) {
		currparser = NULL;
	}

	if (c == '\r') {
		c = '\n';
	}

	// special case: blank line
	if (!currparser && c == '\n') {
		return false;
	}

	if (!currparser) {
		currparser = find_parser(c);
		if (!currparser) {
			//perror("unknown command");
			return false;
		}
		res = currparser->callback(c, true);
	} else {
		res = currparser->callback(c, false);
	}

	// parsing done, clear current parser
	if (res) {
		currparser = NULL;
	}

	return res;
}

// -- help code --

static inline void print_help(void) {
	parser_t *it=rootparser;
	puts("Commands:");
	while (it) {
		printf("  '%c'\t\t%s\n", it->command, it->help);
		it = it->next;
	}
}

static bool help_parser(char c, bool reinit) {
	if (c == '\n') {
		print_help();
		return true;
	}
	return false;
}

static parser_t helpparsers[] = {
	PARSER_INIT('?', "short help", help_parser),
	PARSER_INIT('H', "short help", help_parser)
};

void register_help(void) {
	register_parser(&helpparsers[0]);
	register_parser(&helpparsers[1]);
}

