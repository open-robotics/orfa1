#include <stdint.h>
#include <stdio.h>

#include "cbuf.h"

typedef enum {
	NO_ERROR=0,         //!< no error
	INTERNAL_ERROR,     //!< internal parser error
	INVALID_COMMAND,    //!< unknown command
	INVALID_XDIGIT,     //!< non hex character
	INVALID_DATA,       //!< invalid data
	P_EXPECTED,         //!< 'P' expected (i2c)
} error_code_t;

typedef enum {
	CMD_INIT,       //!< clear cmd buffer
	GET_COMMAND,    //!< get command ( 'S', 'R', 'W', '#', '\r', '\n' )
	COMMENT_EOL,    //!< skip all chars, wait '\r' or '\n', change state to CMD_INIT
	PARSE_I2C,      //!< parse I2C commands ( S<aa><data>[S<aa><data>]P )
	PARSE_READ,     //!< parse read register command ( R<aa><rr>[<ll>] )
	PARSE_WRITE,    //!< parse write register command ( W<aa><rr><data> )
	WAIT_EOL,       //!< skip all chars, wait '\r' or '\n', change state to EXEC_COMMAND
	EXEC_COMMAND    //!< start exec, change state to CMD_INIT
} state_cmd_t;

typedef enum {
	GET_ADDRESS,            //!< get address (both register and i2c machine)
	GET_REGISTER,           //!< get register (register machine)
	GET_LENGTH_OR_EOF,      //!< get length (register machine)
	GET_DATA_OR_EOF,        //!< get data (register machine)
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
	//! temp
	uint8_t c;
	uint8_t cout=0;
	uint8_t data=0;
	uint8_t addr=0;
	uint8_t reg_len=1;
	int8_t tmp;
	
	//! command buffer
	cbf_t cmd_buf;
	
	//! error state
	error_code_t error_code=NO_ERROR;
	
	//! machine states
	state_cmd_t state_cmd=CMD_INIT;
	state_i2creg_t state_i2creg=GET_ADDRESS;

	for(;;)
	{
		c = toupper(getchar());

		if(state_cmd == CMD_INIT)
		{
			// Clear machine
			addr = 0;
			cout = 1;
			reg_len = 1;
			cbf_init(&cmd_buf);
			state_cmd = GET_COMMAND;
			state_i2creg = GET_ADDRESS;
		}

		// ---------------------------------------------------------------

		switch(state_cmd)
		{
		case GET_COMMAND:
			switch(c)
			{
			case 'S':
				// is i2c?
				state_cmd = PARSE_I2C;
				cbf_put(&cmd_buf, 'S');
				break;
			
			case 'R':
				// is read register?
				state_cmd = PARSE_READ;
				cbf_put(&cmd_buf, 'S');
				break;
				
			case 'W':
				// is write register?
				state_cmd = PARSE_WRITE;
				cbf_put(&cmd_buf, 'S');
				break;

			case '\r':
			case '\n':
				break;
				
			case '#':
				// is comment?
				state_cmd = COMMENT_EOL;
				break;
				
			default:
				error_code = INVALID_COMMAND;
				break;
			}
			break;
			
		case COMMENT_EOL:
			// skip line
			if(c == '\r' || c == '\n')
			{
				state_cmd = CMD_INIT;
			}
			break;
		
		case PARSE_I2C:
			switch(state_i2creg)
			{
			case GET_ADDRESS:
				if(get_cmd_byte(c, &data, &error_code))
				{
					cbf_put(&cmd_buf, data);
					state_i2creg = (data & 0x01) ? PARSE_I2C_READ : PARSE_I2C_WRITE;
				}
				break;

			case PARSE_I2C_READ:
				if(get_cmd_byte(c, &data, &error_code))
				{
					cbf_put(&cmd_buf, data);
					state_i2creg = WAIT_RESTART_OR_STOP;
				}
				break;
			
			case WAIT_RESTART_OR_STOP:
				switch(c)
				{
				case 'S':
					// reSTART?
					cbf_put(&cmd_buf, c);
					state_i2creg = GET_ADDRESS;
					break;
					
				case 'P':
					// STOP?
					cbf_put(&cmd_buf, c);
					state_cmd = WAIT_EOL;
					break;
					
				default:
					break;
				}
				break;
			
			case PARSE_I2C_WRITE:
				tmp = xtoi(c);
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
					// get first nibble
					data = tmp << 4;
					cout = 2;
				}
				else if(tmp >= 0 && cout == 2)
				{
					// get second nibble
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
				break;
				
			} // parse_i2c state machine
			break;
			
		case PARSE_READ:
			switch(state_i2creg)
			{
			case GET_ADDRESS:
				if(get_cmd_byte(c, &data, &error_code))
				{
					addr = data & (~0x01);
					cbf_put(&cmd_buf, addr);
					state_i2creg = GET_REGISTER;
				}
				break;
				
			case GET_REGISTER:
				if(get_cmd_byte(c, &data, &error_code))
				{
					cbf_put(&cmd_buf, 1);
					cbf_put(&cmd_buf, data);
					cbf_put(&cmd_buf, 'S');
					cbf_put(&cmd_buf, addr|0x01);
					state_i2creg = GET_LENGTH_OR_EOF;
				}
				break;

			case GET_LENGTH_OR_EOF:
				tmp = xtoi(c);
				if(tmp >= 0 && cout == 1)
				{
					// get first nibble
					reg_len = tmp << 4;
					cout = 2;
				}
				else if(tmp >= 0 && cout == 2)
				{
					// get second nibble
					reg_len |= tmp;
					cout = 1;
				}
				else if(c == ' ' && cout != 2)
				{
					// skip spaces
				}
				else if((c == '\r' || c == '\n') && cout != 2)
				{
					cbf_put(&cmd_buf, reg_len);
					cbf_put(&cmd_buf, 'P');
					cbf_put(&cmd_buf, '\n');
					state_cmd = EXEC_COMMAND;
				}
				break;
				
			default:
				error_code = INTERNAL_ERROR;
				break;
			}			
			break;
			
		case PARSE_WRITE:
			switch(state_i2creg)
			{
			case GET_ADDRESS:
				if(get_cmd_byte(c, &data, &error_code))
				{
					addr = data & (~0x01);
					cbf_put(&cmd_buf, addr);
					state_i2creg = GET_REGISTER;
				}
				break;
				
			case GET_REGISTER:
				if(get_cmd_byte(c, &data, &error_code))
				{
					cbf_put(&cmd_buf, 1);
					cbf_put(&cmd_buf, data);
					state_i2creg = GET_DATA_OR_EOF;
				}
				break;

			case GET_DATA_OR_EOF:
				tmp = xtoi(c);
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
					// get first nibble
					data = tmp << 4;
					cout = 2;
				}
				else if(tmp >= 0 && cout == 2)
				{
					// get second nibble
					data |= tmp;
					cbf_put(&cmd_buf, 1);
					cbf_put(&cmd_buf, data);
					cout = 1;
				}
				else if((c == '\r' || c == '\n') && cout == 1)
				{
					cbf_put(&cmd_buf, 'P');
					cbf_put(&cmd_buf, '\n');
					state_cmd = EXEC_COMMAND;
				}
				else if(c == ' ' && cout == 1)
				{
					// skip spaces
				}
				else
				{
					error_code = INVALID_DATA;
				}
				break;
				
			default:
				error_code = INTERNAL_ERROR;
				break;
			}
			break;
			
		case WAIT_EOL:
			if(c == '\r' || c == '\n')
			{
				// EOL? (exec)
				cbf_put(&cmd_buf, '\n');
				state_cmd = EXEC_COMMAND;
			}
			break;
			
			default:
				error_code = INTERNAL_ERROR;
				break;
		}

		// ---------------------------------------------------------------

		if(state_cmd == EXEC_COMMAND)
		{
			while(!cbf_isempty(&cmd_buf))
			{
				c = cbf_get(&cmd_buf);
				if(c == '\n')
					printf("\\n ");
				else if(c == '\r')
					printf("\\r ");
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
