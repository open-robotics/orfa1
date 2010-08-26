
#include "eterm.h"

bool register_serialgate(void);
bool register_orc32(void);

int main(int argv, char *argc[]) {
	register_serialgate();
	register_orc32();
	register_help();

	while (true) {
		char c = getchar();
		bool res = parse_command(c, false);
	}

	return 0;
}

