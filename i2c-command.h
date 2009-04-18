#ifndef I2C_COMMAND_H
#define I2C_COMMAND_H

#include <stdint.h>
#include <stdbool.h>

#include "i2c.h"
#include "debug.h"
#include "errors.h"

bool cmd_start(uint8_t address, i2c_rdwr_t flag);
void cmd_stop(void);
bool cmd_txc(uint8_t c);
bool cmd_rxc(uint8_t *c, bool ack);


#endif  // I2C_COMMAND_H
