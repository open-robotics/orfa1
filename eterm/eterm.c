
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

