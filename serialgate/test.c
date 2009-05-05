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

int main()
{
	i2c_set_handlers(&cmd_start, &cmd_stop, &cmd_txc, &cmd_rxc);

	serialgate_mainloop();
	
	return 0;
} // main()

