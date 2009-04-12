#include <stdint.h>
#include <stdio.h>

#include "cbuf.h"

typedef enum {
	NO_ERROR=0,         //!< no error
	INVALID_COMMAND,    //!< unknown command
	INVALID_DATA,       //!< invalid data
	INVALID_XDIGIT,     //!< non hex character
	P_EXPECTED,         //!< 'P' expected (i2c)
} error_code_t;

typedef enum {
	CMD_INIT,       //!< clear cmd buffer
	GET_COMMAND,    //!< get command ( 'S', 'R', 'W', '#', '\r', '\n' )
	COMMENT_EOL,    //!< skip all chars, wait '\r' or '\n', change state to CMD_INIT
	PARSE_I2C,      //!< parse I2C commands ( S<aa><data>[S<aa><data>]P )
	PARSE_READ,     //!< parse read register command
	PARSE_WRITE,    //!< parse write register command
	WAIT_EOL,       //!< skip all chars, wait '\r' or '\n', change state to EXEC_COMMAND
	EXEC_COMMAND    //!< start exec, change state to CMD_INIT
} state_cmd_t;

typedef enum {
	GET_ADDRESS,            //!< get address (both register and i2c machine)
	GET_REGISTER,           //!< get register (register machine)
	PARSE_I2C_READ,         //!< parse i2c read, change state to WAIT_RESTART_OR_STOP
	PARSE_I2C_WRITE,        //!< parse i2c write
	WAIT_RESTART_OR_STOP    //!< wait next i2c frame or stop
} state_i2creg_t;


/*!
 * Convert hex to int8 (0 — 15)
 * \param[in] c 0 — 9, A — F
 * \return -1 if fail
 */
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

/*!
 * Get one byte state machine
 * \param[in] c char
 * \param[out] *data received byte
 * \param[out] *error_code error reporting
 * \return true when receive complete
 */
static inline bool get_cmd_byte(uint8_t c, uint8_t *data, error_code_t *error_code)
{
	static uint8_t cout = 1;
	int8_t tmp = xtoi(c);
	if(tmp >= 0 && cout == 1)
	{
		// get first nibble
		*data = tmp << 4;
		cout = 2;
	}
	else if(tmp >= 0 && cout == 2)
	{
		// get second nibble
		*data |= tmp;
		cout = 1;
		return true;
	}
	else if(c == ' ' && cout == 1)
	{
	    // skip spaces
	}
	else
	{
		cout = 1;
		*error_code = INVALID_XDIGIT;
	}
	
	return false;
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
				else if(c == 'S' && cout == 1)
				{
					// reSTART?
					cbf_put(&cmd_buf, c);
					state_i2creg = GET_ADDRESS;
				}
				else if(c == 'P' && cout == 1)
				{
					// STOP?
					cbf_put(&cmd_buf, c);
					state_cmd = WAIT_EOL;
				}
				else if(c == ' ' && cout == 1)
				{
					// skip spaces
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
