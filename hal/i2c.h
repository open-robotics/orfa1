#ifndef I2C_HAL_H
#define I2C_HAL_H

#include "i2c_lld.h"

/** I2C init
 */
#define i2c_init  i2c_lld_init

/** Set I2C start/stop event callbacks
 */
#define i2c_set_evt_handlers  i2c_lld_set_evt_handlers

/** Set I2C slave rx/tx callbacks
 */
#define i2c_set_slave_handlers  i2c_lld_set_slave_handlers

/** Set I2C master rx/tx callbacks
 */
#define i2c_set_master_handlers  i2c_lld_set_master_handlers

/** Set I2C bus speed
 */
#define i2c_set_freq  i2c_lld_set_freq

/** Get current bus speed
 */
#define i2c_get_freq  i2c_lld_get_freq

/** Set self slave address
 */
#define i2c_set_local  i2c_lld_set_local

/** Get current self address
 */
#define i2c_get_local  i2c_lld_get_local

/** Start master transmission (master write)
 */
#define i2c_start_transmission  i2c_lld_start_transmission

/** Start master read
 */
#define i2c_request  i2c_lld_request

#ifdef I2C_NO_ISR
/** I2C controller task
 */
#define i2c_loop  i2c_lld_loop
#endif

#endif // I2C_HAL_H

