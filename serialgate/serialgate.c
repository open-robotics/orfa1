/*
 *  ORFA -- Open Robotics Firmware Architecture
 *
 *  Copyright (c) 2009 Vladimir Ermakov, Andrey Demenev
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 *  THE SOFTWARE.
 *****************************************************************************/

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
