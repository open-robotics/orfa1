
#include "eterm.h"
#include "core/ports.h"
#include "hal/motor.h"

#define ILLIGAL_PORT  100

// -- helpers --

static inline void md2_setspeed(int16_t left, int16_t right)
{
	if(abs(left)>100 || abs(right)>100){
		printf("ERR in DriveLR cmd - only -100..+100 values allowed\n");
		return;
	};
	motor_set_direction(0, (left<0?1:0));
	motor_set_direction(1, (right<0?1:0));
	motor_set_pwm(0,abs(left)*100/255);
	motor_set_pwm(1,abs(right)*100/255);
	printf("Drv(%d,%d)\n",left,right);
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
	static int16_t _valueL=0;
	static int16_t _valueR=0;
	static int16_t _value=0;
	static uint8_t _minus=' ';
	
	if (reinit) {
		// Clear machine
		state_cmd = MCP_GET_COMMAND;
		_cmd = ' ';
		_valueL=999;
		_valueR=999;
		_value=0;
		_minus=' ';
		return false;
	}

	switch (state_cmd) {
		case MCP_GET_COMMAND:
			switch(c) {
				case 'L':
					_cmd = c;
					state_cmd = MCP_GET_L_VALUE;
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
					_value = _value*10+c-'0';
					if(_value>100) state_cmd = MCP_ERROR;
					return false;
				case ' ':
				case '=':
					return false;
				case '\n':
					if(_value_R==999) printf("ERR03 in DrvLR cmd - not enough params\n");
					if(_value_R<999) md2_setspeed(_value_L,_value_R);
					return true;
				case '-':
					_minus='-';
					return false;
				case ',':
					if(_value_R<999){
						printf("ERR04 in DrvLR cmd - to many params\n");
						state_cmd = MCP_ERROR;
						return false;
					};
					if(_value_L==999){
						printf("ERR05 in DrvLR cmd - first param ommited\n");
						state_cmd = MCP_ERROR;
						return false;
					};
					_value_R=_value;
					if(_minus=='-') _value_R=-_value;
					_value=0;
					_minus=' ';
					return false;
				default:
					state_cmd = PCP_ERROR;
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

static parser_t portparsers[] = {
	PARSER_INIT('D', "Drive chassis control", md2_control_parser)
};

void register_port(void) {
	for (uint8_t i=0; i < ARRAY_SIZE(portparsers); i++) {
		register_parser(portparsers + i);
	}
}

