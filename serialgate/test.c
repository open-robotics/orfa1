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

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "i2c.h"
#include "common.h"
#include "serialgate.h"

bool cmd_start(uint8_t address, i2c_rdwr_t flag)
{
	debug("# > cmd_start(0x%02x, %i)\n", address, flag);
	return true;
}

void cmd_stop(void)
{
	debug("# > cmd_stop()\n");
}

bool cmd_txc(uint8_t c)
{
	debug("# > cmd_txc(0x%02x)\n", c);
	return true;
}

bool cmd_rxc(uint8_t *c, bool ack)
{
	debug("# > cmd_rxc(0x%02x, %i)\n", *c, ack);
	return true;
}

int main(void)
{
	serialgate_init();
	i2c_set_handlers(&cmd_start, &cmd_stop, &cmd_txc, &cmd_rxc);

	while(true)
		serialgate_supertask();
	
	return 0;
} // main()

