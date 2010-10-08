
#include "eterm.h"
#include "core/ports.h"
#include "hal/motor.h"

#include <stdlib.h>
#include <math.h>

#define ILLIGAL_PORT  100

// -- helpers --

static inline void md2_setspeed(int16_t left, int16_t right)
{
	if (abs(left) > 100 || abs(right) > 100) {
		printf("ERR in DriveLR cmd - only -100..+100 values allowed\n");
		return;
	}
	motor_set_direction(0, left < 0);
	motor_set_direction(1, right < 0);
	motor_set_pwm(0, abs(left) * 100/255);
	motor_set_pwm(1, abs(right) * 100/255);
	printf("Drv(%d,%d)\n", left, right);
}

// -- parser --

typedef enum {
	MCP_GET_COMMAND,			//0/< get command
	MCP_GET_v,					//1/< get "v" in "Drv"
	MCP_GET_R,					//2/< get "R" in "DrvLR"
	MCP_GET_VALUE,				//3/< get left value
	MCP_WAIT_EOL,				//4/< skip all chars because command end, wait for '\r' or '\n'
	MCP_ERROR,					//5/< skip all chars because command error, wait for '\r' or '\n'
} state_cmd_pcp;

static bool md2_control_parser(char c, bool reinit) {
	static state_cmd_pcp state_cmd;
	static uint8_t _cmd=' ';
	static int16_t val_L=0;
	static int16_t val_R=0;
	static int16_t value=0;
	static bool minux_flag=false;
	
	if (reinit) {
		// Clear machine
		state_cmd = MCP_GET_COMMAND;
		_cmd = ' ';
		val_L = 999;
		val_R = 999;
		value = 0;
		minux_flag = false;
		return false;
	}

	switch (state_cmd) {
		case MCP_GET_COMMAND:
			switch (c) {
				case 'L':
					_cmd = c;
					state_cmd = MCP_GET_VALUE;
					return false;

				case 'r':
					state_cmd = MCP_GET_v;
					return false;

				case '\n':
					printf("ERR01 in Drv cmd - wrong command\n");
					return true;

				default:
					state_cmd = MCP_ERROR;
					return false;
			}
			break;

		case MCP_GET_VALUE:
			switch (c) {
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
					value = value*10+c-'0';
					if (value > 100)
						state_cmd = MCP_ERROR;
					return false;

				case ' ':
				case '=':
					return false;

				case '\n':
					if (val_R == 999)
						printf("ERR03 in DrvLR cmd - not enough params\n");
					if (val_R < 999)
						md2_setspeed(val_L, val_R);
					return true;

				case '-':
					minux_flag = true;
					return false;

				case ',':
					if (val_R < 999) {
						printf("ERR04 in DrvLR cmd - to many params\n");
						state_cmd = MCP_ERROR;
						return false;
					}
					if (val_L == 999) {
						printf("ERR05 in DrvLR cmd - first param ommited\n");
						state_cmd = MCP_ERROR;
						return false;
					}
					val_R = (minux_flag)? -value : value;
					value = 0;
					minux_flag = false;
					return false;

				default:
					state_cmd = MCP_ERROR;
					return false;
			}
			break;

		case MCP_GET_v:
			switch (c) {
				case 'v':
					state_cmd = MCP_GET_COMMAND;
					return false;

				case '\n':
					printf("ERR06 in Drv cmd\n");
					return true;

				default:
					state_cmd = MCP_ERROR;
					return false;
			}
			break;

		default:
			state_cmd = MCP_ERROR;
			break;
		
	}

	if (c != '\n')
		return false;
	printf("ERR09 in Drv cmd\n");
	return true;
}

// -- object --

static parser_t md2parsers[] = {
	PARSER_INIT('D', "Drive chassis control", md2_control_parser)
};

void register_md2(void) {
	for (uint8_t i=0; i < ARRAY_SIZE(md2parsers); i++) {
		register_parser(md2parsers + i);
	}
}

