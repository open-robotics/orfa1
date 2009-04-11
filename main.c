#include <stdint.h>
#include <stdio.h>

#include "cbuf.h"

typedef enum {
	NO_ERROR=0,
	INVALID_COMMAND,
	INVALID_DATA,
	INVALID_XDIGIT,
	P_EXPECTED,
} error_code_t;

typedef enum {
	CMD_INIT,
	GET_COMMAND,
	COMMENT_EOL,
	PARSE_I2C,
	PARSE_READ,
	PARSE_WRITE,
	WAIT_EOL,
	EXEC_COMMAND
} state_cmd_t;

typedef enum {
	GET_ADDRESS,
	GET_REGISTER,
	PARSE_I2C_READ,
	PARSE_I2C_WRITE,
	WAIT_RESTART_OR_STOP
} state_i2creg_t;

int8_t xtoi(uint8_t c)
{
	if((c >= '0')&&(c <= '9'))
	{
		return c - '0';
	}
	else if((c >= 'A')&&(c <= 'F'))
	{
		return c - 'A' + 10;
	}
	else
	{
		return -1;
	}
}

static inline bool get_cmd_byte(uint8_t c, uint8_t *data, error_code_t *error_code)
{
	static uint8_t cout = 1;
	int8_t tmp = xtoi(c);
	if(tmp >= 0 && cout == 1)
	{
		// get first half
		*data = tmp << 4;
		cout = 2;
		return false;
	}
	else if(tmp >= 0 && cout == 2)
	{
		// get second half
		*data |= tmp;
		cout = 1;
		return true;
	}
	else
	{
		cout = 1;
		*error_code = INVALID_XDIGIT;
		return false;
	}
}

int main()
{
	uint8_t c;
	uint8_t cout=0;
	uint8_t data=0;
	error_code_t error_code;
	cbf_t cmd_buf;
	state_cmd_t state_cmd=CMD_INIT;
	state_i2creg_t state_i2creg=GET_ADDRESS;

	for(;;)
	{
		c = toupper(getchar());

		if(state_cmd == CMD_INIT)
		{
			// Init
			cbf_init(&cmd_buf);
			cout = 1;
			state_i2creg = GET_ADDRESS;
			state_cmd = GET_COMMAND;
		}

		// ---------------------------------------------------------------

		if(state_cmd == GET_COMMAND && c == 'S')
		{
			// is i2c?
			cbf_put(&cmd_buf, c);
			state_cmd = PARSE_I2C;
		}
		else if(state_cmd == GET_COMMAND && ((c == '\r')||(c == '\n')))
		{
			// is blank line?
			state_cmd = GET_COMMAND;
		}
		else if(state_cmd == GET_COMMAND && c == '#')
		{
			// is comment?
			state_cmd = COMMENT_EOL;
		}
		else if(state_cmd == COMMENT_EOL)
		{
			// skip line
			if(c == '\r' || c == '\n')
			{
				state_cmd = CMD_INIT;
			}
		}
		else if(state_cmd == PARSE_I2C)
		{
			if(state_i2creg == GET_ADDRESS)
			{
				if(get_cmd_byte(c, &data, &error_code))
				{
					cbf_put(&cmd_buf, data);
					state_i2creg = (data & 0x01) ? PARSE_I2C_READ : PARSE_I2C_WRITE;
				}
			}
			else if(state_i2creg == PARSE_I2C_READ)
			{
				if(get_cmd_byte(c, &data, &error_code))
				{
					cbf_put(&cmd_buf, data);
					state_i2creg = WAIT_RESTART_OR_STOP;
				}
			}
			else if(state_i2creg == WAIT_RESTART_OR_STOP)
			{
				if(c == 'S')
				{
					// reSTART?
					cbf_put(&cmd_buf, c);
					state_i2creg = GET_ADDRESS;
				}
				else if(c == 'P')
				{
					// STOP?
					cbf_put(&cmd_buf, c);
					state_cmd = WAIT_EOL;
				}
			}
			else if(state_i2creg == PARSE_I2C_WRITE)
			{
				int8_t tmp = xtoi(c);
				if(c == '\\' && cout == 1)
				{
					// masked byte?
					cout = '\\';
				}
				else if(cout == '\\')
				{
					// get masked byte
					cbf_put(&cmd_buf, 1);
					cbf_put(&cmd_buf, c);
					cout = 1;
				}
				else if(tmp >= 0 && cout == 1)
				{
					// get first half
					data = tmp << 4;
					cout = 2;
				}
				else if(tmp >= 0 && cout == 2)
				{
					// get second half
					data |= tmp;
					cbf_put(&cmd_buf, 1);
					cbf_put(&cmd_buf, data);
					cout = 1;
				}
				else if(c == 'S')
				{
					// reSTART?
					cbf_put(&cmd_buf, c);
					state_i2creg = GET_ADDRESS;
				}
				else if(c == 'P')
				{
					// STOP?
					cbf_put(&cmd_buf, c);
					state_cmd = WAIT_EOL;
				}
				else
				{
					error_code = INVALID_DATA;
				}
			} // parse_i2c state machine
		}
		else if(state_cmd == WAIT_EOL && (c == '\r' || c == '\n'))
		{
			// EOL? (exec)
			cbf_put(&cmd_buf, '\n');
			state_cmd = EXEC_COMMAND;
		}
		else
		{
			error_code = INVALID_COMMAND;
		} // main state machine

		// ---------------------------------------------------------------

		if(state_cmd == EXEC_COMMAND)
		{
			while(!cbf_isempty(&cmd_buf))
			{
				c = cbf_get(&cmd_buf);
				if(c == '\n')
					printf("\\n ");
				else if(c < '0')
					printf("\\x%02X ", c);
				else
					printf("%c ", c);
			}
			printf("\n");

			state_cmd = CMD_INIT;
		}

		if(error_code != NO_ERROR)
		{
			printf("ERROR: %i\n", error_code);

			error_code = NO_ERROR;
			state_cmd = CMD_INIT;
		}
	} // for

} // main()
