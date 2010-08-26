#ifndef ETERM_H
#define ETERM_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>

/** Parser init
 */
#define PARSER_INIT(_command, _help, _callback) { .command = _command, .help = _help, .callback = _callback }

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

