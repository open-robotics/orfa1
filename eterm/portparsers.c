
#include "eterm.h"
#include "core/ports.h"
#include "hal/adc.h"

#define ILLIGAL_PORT  100

// -- helpers --

static inline void adc_ref(uint8_t _ref)
{
	if (_ref == 'E') {
		adc_config |= 0x00; // External
		printf("AdcRef=Ext\n");
	}
	if (_ref == 'A') {
		adc_config |= 0x01; // AVCC
		printf("AdcRef=AVCC\n");
	}
	if (_ref == 'I') {
		adc_config |= 0x02; // Internal
		printf("AdcRef=Int\n");
	}
	adc_reconfigure(adc_get_mask());
}

static inline void adc_bits(uint8_t _ref)
{
	if (_ref == '1') {
		adc_config |= 0x04; //10 bit
		printf("AdcBits=10\n");
	}
	if (_ref == '8') {
		adc_config &= ~0x04; //8 bit
		printf("AdcBits=8\n");
	}
	adc_reconfigure(adc_get_mask());
}

static inline const uint8_t pcp_adc_port(void)
{
#if defined(OR_AVR_M32_D)
	return 'A';
#elif defined(OR_AVR_M128_S) || defined(OR_AVR_M128_DS)
	return 'F';
#else
#error "Unsupported platform!"
#endif
}

static const uint8_t pcp_port_number(uint8_t _port)
{
#ifdef OR_AVR_M32_D
	switch (_port) {
		case 'A': return 0;
		case 'B': return 1;
		case 'C': return 2;
		case 'D': return 3;
	}
#elif defined(OR_AVR_M128_S) || defined(OR_AVR_M128_DS)
	switch (_port) {
		case 'A': return 0;
		case 'F': return 1;
		case 'B': return 2;
		case 'E': return 3;
	}
#endif

	printf("ERR in P cmd - wrong port id for this controller\n");
	return ILLIGAL_PORT;
}

// -- common --

static inline void pcp_mode(uint8_t _port, uint8_t _pin, uint8_t _mode)
{
	uint8_t _port_num=pcp_port_number(_port);
	if (_port_num == ILLIGAL_PORT)
		return;

	char adc_port=pcp_adc_port();
	if (_port == adc_port) {
		uint8_t adc_mask=adc_get_mask();
		if (_mode == 'A') {
			adc_mask |= (1<<_pin);
		} else {
			adc_mask &= ~(1<<_pin);
		}
		adc_reconfigure(adc_mask);

		if (_mode == 'A') {
			printf("PinMode%c%d=ADC\n", _port, _pin);
			return;
		}
	}

	if (_mode == 'A') {
		printf("ERR in PinMode cmd - only %c port on this controller has ADC function\n", adc_port);
		return;
	} else if (_mode == 'I') {
		gate_port_config(_port_num, 1<<_pin, 0);
		printf("PinMode%c%d=In\n", _port, _pin);
	} else if (_mode == 'O') {
		gate_port_config(_port_num, 1<<_pin, 0xFF);		
		printf("PinMode%c%d=Out\n", _port, _pin);
	}
}

static inline void pcp_set(uint8_t _port, uint8_t _pin, uint8_t _value)
{
	uint8_t _port_num=pcp_port_number(_port);
	if (_port_num == ILLIGAL_PORT)
		return;

	char adc_port=pcp_adc_port();
	if (_port == adc_port) {
		uint8_t adc_mask=adc_get_mask();
		if (adc_mask & (1<<_pin)) {
			printf("ERR in PinSet cmd - write to ADC is prohibited\n");
			return;
		}
	}

	_value -= '0';
	if (_value)
		_value = 0xFF;

	gate_port_write(_port_num, 1<<_pin, _value);
	printf("%c%d=%d\n", _port, _pin, _value > 0);
}

static inline void pcp_get(uint8_t _port, uint8_t _pin)
{
	uint8_t _port_num=pcp_port_number(_port);
	if (_port_num == ILLIGAL_PORT)
		return;

	char adc_port=pcp_adc_port();
	if (_port == adc_port) {
		uint8_t adc_mask=adc_get_mask();
		if (adc_mask & (1<<_pin)) {
			uint32_t value=adc_get_result(_pin);
			
			value = 330*value; //*3.3V*100

			if (adc_is_10bit()) {
				// 10bit
				value = value / 1023; //1024; // div by power of 2 is faster
			} else {
				// 8bit
				value = value / 255; //256;
			}

			printf("%c%d:%d.%02d\n", _port, _pin,
					(uint8_t) (value/100), (uint8_t) (value%100));
			return;
		}
	}

	uint8_t result;
	gate_port_read(_port_num, &result);
	printf("%c%d:%d\n", _port, _pin, (result & (1<<_pin)) > 0);
}

// -- parser --

typedef enum {
	PCP_GET_COMMAND,			//0/< get command
	PCP_GET_n,					//1/< get "n" in "Pin"
	PCP_GET_t,					//2/< get "t" in "PinGet"
	PCP_GET_d,					//3/< get "d" in "PinMode"
	PCP_GET_e,					//4/< get "e" in "PinMode"
	PCP_GET_PORT,				//5/< get port id
	PCP_GET_PIN,				//6/< get pin id
	PCP_GET_VALUE,				//7/< get value
	PCP_WAIT_EOL,				//8/< skip all chars because command end, wait for '\r' or '\n'
	PCP_ERROR,					//9/< skip all chars because command error, wait for '\r' or '\n'
} state_cmd_pcp;

static bool pin_control_parser(char c, bool reinit) {
	static state_cmd_pcp state_cmd;
	static uint8_t _cmd=' ';
	static uint8_t _port=' ';
	static uint8_t _value=' ';
	static uint8_t _pin=' ';
	
	if (reinit) {
		// Clear machine
		state_cmd = PCP_GET_COMMAND;
		_port = ' ';
		_pin = ' ';
		_value = ' ';
		_cmd = ' ';
		return false;
	}

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
					printf("ERR01 in P cmd - wrong command\n");
					return true;

				default:
					state_cmd = PCP_ERROR;
					return false;
			}
			break;

		case PCP_GET_PORT:
			switch (c) {
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
					printf("ERR02 in P cmd - wrong port\n");
					return true;

				default:
					state_cmd = PCP_ERROR;
					return false;
			}
			break;

		case PCP_GET_PIN:
			switch (c) {
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
					_pin = c;
					state_cmd = PCP_GET_VALUE;
					if (_cmd == 'G')
						state_cmd = PCP_WAIT_EOL;
					return false;

				case '\n':
					printf("ERR03 in P cmd - wrong pin\n");
					return true;

				default:
					state_cmd = PCP_ERROR;
					return false;
			}
			break;

		case PCP_GET_VALUE:
			switch (c) {
				case ' ':
				case '=':
					return false;

				case '0':
				case '1':
					if (_cmd != 'S') {
						state_cmd = PCP_ERROR;
						return false;
					}
					_value = c;
					state_cmd = PCP_WAIT_EOL;
					return false;

				case 'I':
				case 'O':
				case 'A':
					if (_cmd != 'M') {
						state_cmd = PCP_ERROR;
						return false;
					}
					_value = c;
					state_cmd = PCP_WAIT_EOL;
					return false;

				case '\n':
					printf("ERR04 in P cmd - wrong value\n");
					return true;

				default:
					state_cmd = PCP_ERROR;
					return false;
			}
			break;

		case PCP_WAIT_EOL:
			switch (c) {
				case ' ':
					return false;

				case '\n':
					if (_cmd == 'M') {
						pcp_mode(_port, _pin - '0', _value);
						return true;
					}
					if (_cmd == 'S') {
						pcp_set(_port, _pin - '0', _value);
						return true;
					}
					if (_cmd == 'G') {
						pcp_get(_port, _pin - '0');
						return true;
					}
					return true;
				default:
					state_cmd = PCP_ERROR;
					return false;
			}
			break;

		
		case PCP_GET_n:
			switch (c) {
				case 'n':
					state_cmd = PCP_GET_COMMAND;
					return false;

				case '\n':
					printf("ERR05 in Pin cmd\n");
					return true;

				default:
					state_cmd = PCP_ERROR;
					return false;
			}
			break;

		case PCP_GET_t:
			switch (c) {
				case 't':
					state_cmd = PCP_GET_PORT;
					return false;

				case '\n':
					printf("ERR06 in PinGet cmd\n");
					return true;

				default:
					state_cmd = PCP_ERROR;
					return false;
			}
			break;

		case PCP_GET_d:
			switch (c) {
				case 'd':
					state_cmd = PCP_GET_e;
					return false;

				case '\n':
					printf("ERR07 in PinMode cmd\n");
					return true;

				default:
					state_cmd = PCP_ERROR;
					return false;
			}
			break;

		case PCP_GET_e:
			switch (c) {
				case 'e':
					state_cmd = PCP_GET_PORT;
					return false;

				case '\n':
					printf("ERR08 in PinMode cmd\n");
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
	printf("ERR09 in P cmd\n");
	return true;
}

typedef enum {
	ACP_GET_COMMAND,			//0/< get command
	ACP_GET_c,					//1/< get "c" in "Adc"
	ACP_GET_f,					//2/< get "f" in "Ref"
	ACP_GET_t,					//2/< get "t" in "Bits"
	ACP_GET_s,					//2/< get "s" in "Bits"
	ACP_GET_VALUE,				//3/< get value
	ACP_WAIT_EOL,				//4/< skip all chars because command end, wait for '\r' or '\n'
	ACP_ERROR					//5/< skip all chars because command error, wait for '\r' or '\n'
} state_cmd_acp;

static bool adc_config_parser(char c, bool reinit) {
	static state_cmd_acp state_cmd;
	static uint8_t _cmd=' ';
	static uint8_t _value=' ';
	
	if (reinit) {
		// Clear machine
		state_cmd = ACP_GET_COMMAND;
		_value = ' ';
		_cmd=' ';
		return false;
	}

	switch (state_cmd) {
		case ACP_GET_COMMAND:
			switch(c) {
				case 'R':
				case 'B':
					_cmd = c;
					state_cmd = ACP_GET_VALUE;
					return false;

				case 'd':
					state_cmd = ACP_GET_c;
					return false;

				case '\n':
					printf("ERR01 in A cmd - wrong command\n");
					return true;

				default:
					state_cmd = ACP_ERROR;
					return false;
			}
			break;

		case ACP_GET_VALUE:
			switch (c) {
				case ' ':
				case '=':
					return false;

				case '8':
				case '1':
					if (_cmd != 'B') {
						state_cmd = ACP_ERROR;
						return false;
					}
					_value = c;
					state_cmd = ACP_WAIT_EOL;
					return false;

				case 'I':
				case 'E':
				case 'A':
					if (_cmd != 'R') {
						state_cmd = ACP_ERROR;
						return false;
					}
					_value = c;
					state_cmd = ACP_WAIT_EOL;
					return false;

				case 'e': //Ref?
					state_cmd = ACP_GET_f;
					return false;

				case 'i': //Bits?
					state_cmd = ACP_GET_t;
					return false;

				case '\n':
					printf("ERR04 in A cmd - wrong value\n");
					return true;

				default:
					state_cmd = ACP_ERROR;
					return false;
			}
			break;

		case ACP_WAIT_EOL:
			switch (c) {
				case ' ':
					return false;

				case '\n':
					if (_cmd == 'R') {
						adc_ref(_value);
					}
					if (_cmd == 'B') {
						adc_bits(_value);
					}
					return true;

				default:
					state_cmd = ACP_ERROR;
					return false;
			}
			break;

		
		case ACP_GET_f:
			switch (c) {
				case 'f':
					state_cmd = ACP_GET_VALUE;
					return false;

				case '\n':
					printf("ERR06 in AdcRef cmd\n");
					return true;

				default:
					state_cmd = ACP_ERROR;
					return false;
			}
			break;

		case ACP_GET_t:
			switch (c) {
				case 't':
					state_cmd = ACP_GET_s;
					return false;

				case '\n':
					printf("ERR07 in AdcBits cmd\n");
					return true;

				default:
					state_cmd = ACP_ERROR;
					return false;
			}
			break;

		case ACP_GET_s:
			switch (c) {
				case 's':
					state_cmd = ACP_GET_VALUE;
					return false;

				case '\n':
					printf("ERR08 in AdcBits cmd\n");
					return true;

				default:
					state_cmd = ACP_ERROR;
					return false;
			}
			break;

		default:
			state_cmd = ACP_ERROR;
			break;
		
	}

	if (c != '\n')
		return false;
	printf("ERR09 in A cmd\n");
	return true;
}

// -- object --

static parser_t portparsers[] = {
	PARSER_INIT('P', "Pin control", pin_control_parser),
	PARSER_INIT('A', "ADC config", adc_config_parser)
};

void register_port(void) {
	for (uint8_t i=0; i < ARRAY_SIZE(portparsers); i++) {
		register_parser(portparsers + i);
	}
}

