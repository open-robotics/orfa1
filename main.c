#include <stdint.h>
#include <stdio.h>

#include "cbuf.h"

typedef enum {
	INVALID_DATA,
	P_EXPECTED,
} error_code_t;

typedef enum {
	get_command,
	process_i2c,
	process_read,
	process_write,
	process_request,
	error_reporting
} state_cmd_t;

typedef enum {
	get_address,
	get_data,
} state_i2c_t;

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

int main()
{
	uint8_t c;
	uint8_t cout;
	uint8_t data;
	error_code_t error_code;
	cbf_t cmd_buf;
	state_cmd_t state_cmd=get_command;
	state_i2c_t state_i2c=get_address;

	for(c=toupper(getchar());;)
	{
		if(state_cmd == get_command && ((c == 'S')||(c == 'R')||(c == 'W')))
		{
			cbf_init(&cmd_buf);
			cout = 1;
			cbf_put(&cmd_buf, 'S');
			state_cmd = process_i2c;
			state_i2c = get_address;
		}
		else if(state_cmd == process_i2c)
		{
			if(state_i2c == get_address)
			{
				int8_t tmp = xtoi(c);
				if(tmp >= 0 && cout == 1)
				{
					// get first 4 bits
					data = tmp << 4;
					cout = 2;
				}
				else if(tmp >= 0 && cout == 2)
				{
					// get second 4 bits
					data |= tmp;
					cout = 1;
					cbf_put(&cmd_buf, data);
					state_i2c = get_data;
				}
				else
				{
					// error
					state_cmd = error_reporting;
					error_code = 1;
				}
			}
			else if(state_i2c == get_data)
			{
				int8_t tmp = xtoi(c);
				if(c == '\\' && count == 1)
				{
					// masked byte?
					count = '\\';
				}
				else if(count == '\\')
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
				else if(c == 'S' || c == 'P')
				{
					// reSTART or STOP?
					cbf_put(&cmd_buf, c);
				}
				else if(c == '\r' || c == '\n')
				{
					state_cmd = process_request;
				}
				else
				{
					state_cmd = error_reporting;
					error_code = 2;
				}
			} // process_i2c
		}
	}
}
