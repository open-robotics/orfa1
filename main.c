
#include <stdint.h>
#include <stdio.h>

#include "serialgate/i2c.h"
#include "serialgate/common.h"
#include "serialgate/serialgate.h"

typedef enum {
    GET_REGISTER,
    GET_DATA,
} state_i2c_t;

static state_i2c_t state_i2c = GET_REGISTER;
static uint8_t register_addr = 0x00;


/*! Handle I2C Start event
 * \param[in] address device address
 * \param[in] flar Write/Read flag
 * \return true if success (always)
 */
bool cmd_start(uint8_t address, i2c_rdwr_t flag)
{
    debug("# > cmd_start(0x%02x, %i)\n", address, flag);
    state_i2c = GET_REGISTER;

    return true;
}

/*! Handle I2C Stop event
 */
void cmd_stop(void)
{
    debug("# > cmd_stop()\n");
}

/*! Handle I2C master write (slave receiver)
 * \param[in] c writed byte
 * \return true if success
 */
bool cmd_txc(uint8_t c)
{
    switch(state_i2c)
    {
        case GET_REGISTER:
            register_addr = c;
            state_i2c = GET_DATA;
            break;

        case GET_DATA:
            {
            }
            break;
    };

    debug("# > cmd_txc(0x%02x)\n", c);
    return true;
}

/*! Handle I2C master read (slave transmitter)
 * \param[out] *c byte for read
 * \param[in] ack ack/nack
 * \return true if success
 */
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
}
