/** Realization of WDT-command
 * @file wdtparser.c
 *
 * @author Mad
 */

#include <avr/io.h>
#include "core/wdt_ext.h"
#include "eterm.h"


void proceed_w_command(char command, uint16_t timeout) {

	switch (command)
	{
		case 'N':	// disable connect-watchdog
			{
				wdt_disable_extc();
				printf("W=N\n");
			} break;

		case 'R':	// enable connect-watchdog
			{
				uint16_t	real_timeout;
				uint8_t		wdp_code;

				if (timeout >= 2000) {
					real_timeout = 2000;
					wdp_code = WDTO_2S;
				}
				else if (timeout > 1000) {
					real_timeout = 2000;
					wdp_code = WDTO_2S;
				}
				else if (timeout > 500) {
					real_timeout = 1000;
					wdp_code = WDTO_1S;
				}
				else if (timeout > 250) {
					real_timeout = 500;
					wdp_code = WDTO_500MS;
				}
				else if (timeout > 120) {
					real_timeout = 250;
					wdp_code = WDTO_250MS;
				}
				else if (timeout > 60) {
					real_timeout = 120;
					wdp_code = WDTO_120MS;
				}
				else if (timeout > 30) {
					real_timeout = 60;
					wdp_code = WDTO_60MS;
				}
				else if (timeout > 15) {
					real_timeout = 30;
					wdp_code = WDTO_30MS;
				}
				else {
					real_timeout = 15;
					wdp_code = WDTO_15MS;
				}

				wdt_enable_extc(wdp_code);
				printf("W=R,%i\n", real_timeout);
			}
	}
}
// ---------------------------------------------------------------------------------


typedef enum {
	WP_WAIT_EQ,
	WP_GET_COMMAND,
	WP_WAIT_COMMA,
	WP_WAIT_TIMEOUT,
	WP_GET_TIMEOUT,
	WP_WAIT_EOL,
	WP_SKIP_TO_EOL
} state_cmd_wp;

static bool w_parser(char c, bool reinit) {

	static state_cmd_wp	state_cmd;
	static char			w_cmd;
	static uint16_t		w_timeout;

	if (reinit) {
		// clear
		state_cmd = WP_WAIT_EQ;
		w_cmd = 'N';
		w_timeout = 0;
		return false;
	}

	c = toupper(c);

	switch(state_cmd) {

		case WP_WAIT_EQ:
			switch (c) {
				case ' ':
					return false;

				case '=':
					state_cmd = WP_GET_COMMAND;
					return false;

				default:
					printf("W Error. No '='\n");
					return true;
			} break;

		case WP_GET_COMMAND:
			switch (c) {
				case ' ':
					return false;

				case 'C':
					w_cmd = 'C';
					state_cmd = WP_SKIP_TO_EOL;
					return false;

				case 'N':
					w_cmd = 'N';
					state_cmd = WP_SKIP_TO_EOL;
					return false;

				case 'R':
					w_cmd = 'R';
					state_cmd = WP_WAIT_COMMA;
					return false;

				default:
					printf("W Error. Unknown command\n");
					return true;
			} break;

		case WP_WAIT_COMMA:
			switch (c) {
				case ' ':
					return false;

				case ',':
					state_cmd = WP_WAIT_TIMEOUT;
					w_timeout = 0;
					return false;

				default:
					printf("W Error. Invalid format\n");
					return true;
			} break;

		case WP_WAIT_TIMEOUT:
			switch(c) {
				case ' ':
					return false;

				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
					w_timeout = c - '0';
					state_cmd = WP_GET_TIMEOUT;
					return false;

				default:
					printf("W Error. Invalid timeout\n");
					return true;
			} break;

		case WP_GET_TIMEOUT:
			switch(c) {
				case ' ':
					state_cmd = WP_WAIT_EOL;
					return false;

				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
					w_timeout = w_timeout * 10 + c - '0';
					return false;

				case '\n':
					proceed_w_command(w_cmd, w_timeout);
					return true;

				default:
					printf("W Error. Invalid timeout\n");
					return true;
			} break;

		case WP_WAIT_EOL:
			switch(c) {
				case ' ':
					return false;

				case '\n':
					proceed_w_command(w_cmd, w_timeout);
					return true;

				default:
					printf("W Error. Invalid format\n");
					return true;
			} break;

		case WP_SKIP_TO_EOL:
			if(c != '\n')
				return false;

			proceed_w_command(w_cmd, w_timeout);
			return true;
	}

	return false;
}

/** eTerm 'N' command parser
 *
 * Reset connection watchdog.
 */
static bool n_parser(char c, bool reinit) {
	if (c == '\n') {
		wdt_reset_extc();
		puts("N");
		return true;
	}
	return false;
}

// -- table --
static parser_t wdtparsers[] = {
	PARSER_INIT('W', "WDT-command", w_parser),
	PARSER_INIT('N', "Reset connection watchdog", n_parser)
};

void register_wdt(void) {
	for (uint8_t i=0; i < ARRAY_SIZE(wdtparsers); i++) {
		register_parser(wdtparsers + i);
	}
}

