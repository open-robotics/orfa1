
#include "i2c-command.h"

bool cmd_start(uint8_t address, i2c_rdwr_t flag)
{
    debug("# ---> cmd_start(0x%02x, %i)\n", address, flag);
    return true;
}

void cmd_stop(void)
{
    debug("# ---> cmd_stop()\n");
}

bool cmd_txc(uint8_t c)
{
    debug("# ---> cmd_txc(0x%02x)\n", c);
    return true;
}

bool cmd_rxc(uint8_t *c, bool ack)
{
    debug("# ---> cmd_start(0x%02x, %i)\n", *c, ack);
    return true;
}
