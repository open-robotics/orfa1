#include "serialgate.h"
#include "common.h"
#include "i2c.h"
#include "cbuf.h"
#include "usart.h"
#include "parser.h"
#include "command.h"

#include <stdint.h>
#include <stdio.h>

void serialgate_mainloop(void)
{
	uint8_t c;

	//! command buffer
	cbf_t cmd_buf, tx_buf;

	//! error state
	error_code_t error_code=NO_ERROR;

	cbf_init(&cmd_buf);
	cbf_init(&tx_buf);

	#ifdef AVR_IO
	usart_init(BAUD);
	stdin = stdout = stderr = &usart_fdev;
	#endif // AVR_IO

	i2c_init();

	for(;;)
	{
		c = getchar();

		if(parse_cmd(c, &cmd_buf, &error_code))
		{
			if(exec_cmd(&cmd_buf, &tx_buf, &error_code))
			{
				while(!cbf_isempty(&tx_buf))
				{
					putchar(cbf_get(&tx_buf));
				}
			}
		}

		if(error_code != NO_ERROR)
		{
			print_error(error_code);
			cbf_init(&tx_buf);
			error_code = NO_ERROR;
		}
	} // for
}
