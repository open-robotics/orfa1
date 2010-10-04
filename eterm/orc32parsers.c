
/** ORC32 parsers
 * Parsers list:
 *   - '#' -- set position
 */

#include "eterm.h"
#include "hal/servo.h"
#include "core/ports.h"
#include "hal/adc.h"

/// Debug print
#ifndef NDEBUG
#include <stdio.h>
#define debug(...) printf(__VA_ARGS__)
#else
#define debug(...)
#endif

uint8_t pcp_adc_port(void)
{
#ifdef OR_AVR_M32_D
	return 'A';
#endif

#ifdef OR_AVR_M128_S
	return 'F';
#endif

#ifdef OR_AVR_M128_DS
	return 'F';
#endif
};

uint8_t pcp_port_number(uint8_t _port)
{
#ifdef OR_AVR_M32_D
	switch( _port){
		case 'A': return 0;
		case 'B': return 1;
		case 'C': return 2;
		case 'D': return 3;
	};
#endif

#ifdef OR_AVR_M128_S
	switch( _port){
		case 'A': return 0;
		case 'F': return 1;
		case 'B': return 2;
		case 'E': return 3;
	};
#endif

#ifdef OR_AVR_M128_DS
	switch( _port){
		case 'A': return 0;
		case 'F': return 1;
		case 'B': return 2;
		case 'E': return 3;
	};
#endif
printf("ERR in P cmd - wrong port id for this controller\n");
return 100;
};

void pcp_mode(uint8_t _port, uint8_t _pin, uint8_t _mode)
{
	uint8_t _port_num=pcp_port_number(_port);
	if(_port_num==100) return;
	char adc_port=pcp_adc_port();
	if(_port==adc_port){
		uint8_t adc_mask=adc_get_mask();
		if(_mode=='A'){
			adc_mask|=(1<<_pin);
		}else{
			adc_mask &= (1<<_pin) ^ 0xFF;
		};
		adc_reconfigure(adc_mask);
		if(_mode=='A') return;
	};

	if(_mode=='A'){
		printf("ERR in PinMode cmd - only %c port on this controller has ADC function\n",adc_port);
		return;
	}else if(_mode=='I'){
		gate_port_config(_port_num, 1<<_pin, 0);
	}else if(_mode=='O'){
		gate_port_config(_port_num, 1<<_pin, 0xFF);		
	};
}

void pcp_set(uint8_t _port, uint8_t _pin, uint8_t _value)
{
	uint8_t _port_num=pcp_port_number(_port);
	if(_port_num==100) return;
	char adc_port=pcp_adc_port();
	if(_port==adc_port){
		uint8_t adc_mask=adc_get_mask();
		if(adc_mask&(1<<_pin)){
			printf("ERR in PinSet cmd - write to ADC is prohibited\n");
			return;
		};
	};
	_value-='0';
	if(_value) _value=0xFF;
	gate_port_write(_port_num, 1<<_pin, _value);
}

void pcp_get(uint8_t _port, uint8_t _pin)
{
	uint8_t _port_num=pcp_port_number(_port);
	if(_port_num==100) return;
	char adc_port=pcp_adc_port();
	if(_port==adc_port){
		uint8_t adc_mask=adc_get_mask();
		if(adc_mask&(1<<_pin)){
			uint32_t value=adc_get_result(_pin);
			
			value=330*value; //*3.3V*100

			if( adc_is_10bit() ){
				//10bit
				value=value>>10;
			}else{
				//8bit
				value=value>>8;
			};
			uint8_t volts=value/100;
			uint8_t centivolts=value%100;
			printf("%c%c:%d.%02d\n",_port,_pin,volts,centivolts);
			return;
		};
	};
	uint8_t result=0;
	gate_port_read(_port_num, &result);
	if(result & (1<<_pin)){
		_pin+='0';
		printf("%c%c:1\n",_port,_pin);
	}else{
		_pin+='0';
		printf("%c%c:0\n",_port,_pin);
	};
}

typedef enum {
	PCP_GET_COMMAND,			///< get command
	PCP_GET_n,					///< get "n" in "Pin"
	PCP_GET_t,					///< get "t" in "PinGet"
	PCP_GET_d,					///< get "d" in "PinMode"
	PCP_GET_e,					///< get "e" in "PinMode"
	PCP_GET_PORT,				///< get port id
	PCP_GET_PIN,				///< get pin id
	PCP_GET_VALUE,				///< get value
	PCP_WAIT_EOL,				///< skip all chars because command end, wait for '\r' or '\n'
	PCP_ERROR,					///< skip all chars because command error, wait for '\r' or '\n'
} state_cmd_pcp;

static bool pin_control_parser(char c, bool reinit) {
	static state_cmd_pcp state_cmd;
	static uint8_t _cmd=' ';
	static uint8_t _port=' ';
	static uint8_t _mode=' ';
	static uint8_t _value=' ';
	static uint8_t _pin=0;
	
	if (reinit) {
		// Clear machine
		state_cmd = PCP_GET_COMMAND;
		_port = ' ';
		_pin=0;
		_value = ' ';
		_mode = ' ';
		_cmd=' ';
		return false;
	}

	//printf("%% pcp c=%c init=%d state=%d\n", c, reinit, state_cmd);

	switch (state_cmd) {
		case PCP_GET_COMMAND:
			switch(c) {
				case 'M':
				case 'G':
				case 'S':
					_cmd = c;
					state_cmd = PCP_GET_PORT;
					return false;
				case 'i':
					state_cmd = PCP_GET_n;
					return false;
				case '\n':
					printf("ERR in P cmd - wrong command\n");
					return true;
				default:
					state_cmd = PCP_ERROR;
					return false;
			}
			break;

		case PCP_GET_PORT:
			switch(c) {
				case 'A':
				case 'B':
				case 'C':
				case 'D':
				case 'E':
				case 'F':
					_port = c;
					state_cmd = PCP_GET_PIN;
					return false;
				case 'e':
					state_cmd = PCP_GET_t;
					return false;
				case 'o':
					state_cmd = PCP_GET_d;
					return false;
				case '\n':
					printf("ERR in P cmd - wrong port\n");
					return true;
				default:
					state_cmd = PCP_ERROR;
					return false;
			}
			break;

		case PCP_GET_PIN:
			switch(c) {
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
					_pin = c-'0';
					state_cmd = PCP_GET_VALUE;
					if(_cmd == 'G') state_cmd = PCP_WAIT_EOL;
					return false;
				case '\n':
					printf("ERR in P cmd - wrong pin\n");
					return true;
				default:
					state_cmd = PCP_ERROR;
					return false;
			}
			break;

		case PCP_GET_VALUE:
			switch(c) {
				case ' ':
					return false;
				case '=':
					return false;
				case '0':
				case '1':
					if(_cmd!='S'){
						state_cmd = PCP_ERROR;
						return false;
					};
					_value=c;
					state_cmd = PCP_WAIT_EOL;
					return false;
				case 'I':
				case 'O':
				case 'A':
					if(_cmd!='M'){
						state_cmd = PCP_ERROR;
						return false;
					};
					_value=c;
					state_cmd = PCP_WAIT_EOL;
					return false;
				case '\n':
					printf("ERR in P cmd - wrong value\n");
					return true;
				default:
					state_cmd = PCP_ERROR;
					return false;
			}
			break;

		case PCP_WAIT_EOL:
			switch(c) {
				case ' ':
					return false;
				case '\n':
					if(_cmd=='M'){
						pcp_mode(_port,_pin,_mode);
						return true;
					};
					if(_cmd=='S'){
						pcp_set(_port,_pin,_value);
						return true;
					};
					if(_cmd=='G'){
						pcp_get(_port,_pin);
						return true;
					};
					return true;
				default:
					state_cmd = PCP_ERROR;
					return false;
			}
			break;

		
		case PCP_GET_n:
			switch(c) {
				case 'n':
					_cmd = c;
					state_cmd = PCP_GET_COMMAND;
					return false;
				case '\n':
					printf("ERR in Pin cmd\n");
					return true;
				default:
					state_cmd = PCP_ERROR;
					return false;
			}
			break;

		case PCP_GET_t:
			switch(c) {
				case 't':
					_cmd = c;
					state_cmd = PCP_GET_PORT;
					return false;
				case '\n':
					printf("ERR in PinGet cmd\n");
					return true;
				default:
					state_cmd = PCP_ERROR;
					return false;
			}
			break;

		case PCP_GET_d:
			switch(c) {
				case 'd':
					_cmd = c;
					state_cmd = PCP_GET_e;
					return false;
				case '\n':
					printf("ERR in PinMode cmd\n");
					return true;
				default:
					state_cmd = PCP_ERROR;
					return false;
			}
			break;

		case PCP_GET_e:
			switch(c) {
				case 'e':
					_cmd = c;
					state_cmd = PCP_GET_PORT;
					return false;
				case '\n':
					printf("ERR in PinMode cmd\n");
					return true;
				default:
					state_cmd = PCP_ERROR;
					return false;
			}
			break;

		default:
			state_cmd = PCP_ERROR;
			break;
		
	}

	if (c != '\n')
		return false;
	printf("ERR in P cmd\n");
	return true;
}


typedef enum {
	SMP_GET_COMMAND,			///< get command ( '#', 'P', 'S', 'T', '\r', '\n' )
	SMP_ERROR,					///< skip all chars because command error, wait for '\r' or '\n'
	SMP_PARSE_NUMBER,			///< parse number after command
} state_cmd_smp;

static bool servo_move_parser(char c, bool reinit) {
	static state_cmd_smp state_cmd;
	static uint16_t _servo_target[SERVO_LEN];
	static uint16_t _servo_maxspeed[SERVO_LEN];
	static uint16_t _time2go;
	static uint16_t _servo=0;
	static uint16_t _num=0;
	static uint8_t _cmd=' ';
	
	if (reinit) {
		// Clear machine
		for (uint8_t i=0; i < SERVO_LEN; i++) {
			_servo_target[i] = 0;
			_servo_maxspeed[i] = 0;
		}
		state_cmd = SMP_GET_COMMAND;
		_time2go = 0;
		_cmd = ' ';
		_servo = 0;
		_num = 0;
	}

	//printf("%% smp c=%c init=%d state=%d\n", c, reinit, state_cmd);

	switch (state_cmd) {
		case SMP_PARSE_NUMBER:
			if (c >= '0' && c <= '9') {
				_num = _num*10 + (c - '0');
				state_cmd = SMP_PARSE_NUMBER;
			} else {
				if (_cmd == '#') {
					if(_num < SERVO_LEN)
						_servo = _num;
				} else if (_cmd == 'P') {
					if (_num <= 2500 && _num >= 500) {
						_servo_target[_servo] = _num;
						debug("%% pos[%d]=%d\n", _servo, _num);
					}
				} else if (_cmd == 'S') {
					_servo_maxspeed[_servo] = _num;
					debug("%% spd[%d]=%d\n", _servo, _num);
				} else if (_cmd == 'T') {
					_time2go = _num;
					debug("%% time=%d\n", _num);
				}
				state_cmd = SMP_GET_COMMAND;
			}

		if (state_cmd != SMP_GET_COMMAND)
			break;

		case SMP_GET_COMMAND:
			switch(c) {
				case '#':
				case 'P':
				case 'S':
				case 'T':
					_cmd = c;
					_num = 0;
					state_cmd = SMP_PARSE_NUMBER;
					return false;

				case ' ':
					return false;

				case '\n':
					if (_cmd != ' ')
						servo_command(_time2go, _servo_target, _servo_maxspeed);
					return true;

				default:
					state_cmd = SMP_ERROR;
					return false;
			}
			break;

		default:
			state_cmd = SMP_ERROR;
			break;
		
	}

	if (c != '\n')
		return false;
	printf("ERR in # cmd\n");
	return true;
}


#define QSP_SELECT_CMD  100
#define QSP_Q_ERROR     101
#define QSP_QP_SN_ERROR 102

static bool query_status_parser(char c, bool reinit) {
	static uint8_t servo_num;

	if (reinit) {
		servo_num = QSP_SELECT_CMD;
		return false;
	}

	if (servo_num == QSP_SELECT_CMD) {
		if (c == '\n') {
			if (servo_is_done()) {
				printf(".");
			} else {
				printf("+");
			}
			return true;
		}
		if (c == 'P') {
			servo_num = 0;
			return false;
		}
		if (c != ' ')
			servo_num = QSP_Q_ERROR;
		return false;
	}

	if (servo_num == QSP_Q_ERROR) {
		if (c != '\n')
			return false;
		printf("ERR in Q cmd\n");
		return true;
	}
	
	if (servo_num == QSP_QP_SN_ERROR) {
		if (c != '\n')
			return false;
		printf("ERR in QP cmd servo #\n");
		return true;
	}

	if (servo_num < 100) {
		if (c >= '0' && c <= '9') {
			if (servo_num >= 10) {
				servo_num = QSP_QP_SN_ERROR;
				return false;
			}
			servo_num = servo_num * 10 + (c - '0');
			return false;
		}

		if (c == '\n') {
			char r = servo_get_position(servo_num)/10;
			putchar(r);
			return true;
		}

		if (c != ' ')
			servo_num = QSP_QP_SN_ERROR;
		return false;
	}

	return c == '\n';
}

static parser_t orc32parsers[] = {
	PARSER_INIT('#', "SSC-32 servo move", servo_move_parser),
	PARSER_INIT('Q', "SSC-32 query global status", query_status_parser),
	PARSER_INIT('P', "Pin control", pin_control_parser)
};

void register_orc32(void) {
	for (uint8_t i=0; i < ARRAY_SIZE(orc32parsers); i++) {
		register_parser(orc32parsers + i);
	}
}

