
#include <stdint.h>
#include <stdio.h>

#include "serialgate/i2c.h"
#include "serialgate/serialgate.h"
#include "i2c-command.h"

int main()
{
    i2c_set_handlers(&cmd_start, &cmd_stop, &cmd_txc, &cmd_rxc);

    serialgate_mainloop();
} // main()
