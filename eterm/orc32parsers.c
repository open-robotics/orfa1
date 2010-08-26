
/** ORC32 parsers
 * Parsers list:
 *   - '#' -- set position
 */

#include "eterm.h"

static bool servo_parser(char c, bool reinit) {
	uint8_t servo;
	uint16_t position;
	uint16_t speed;
	uint16_t time;

	return c == '\n';
}

static parser_t orc32parsers[] = {
	PARSER_INIT('#', "SSC-32 servo", servo_parser)
};

void register_orc32(void) {
	register_parser(&orc32parsers[0]);
}

