#ifndef I2C_H
#define I2C_H

#include <stdint.h>
#include <stdbool.h>

// statuses
#define I2C_E_OK		0
#define I2C_E_ARB		1
#define I2C_E_ADDR_NACK	2
#define I2C_E_DATA_NACK	3
#define I2C_E_BUS		4
#define I2C_E_BUFSIZE	5


typedef bool (*i2cRxHandler)(uint8_t);
typedef bool (*i2cTxHandler)(uint8_t*, bool*);

typedef bool (*i2cStartHandler)(uint8_t flag);
typedef void (*i2cStopHandler)(void);


void i2c_lld_init(void);
void i2c_lld_init_slave(uint8_t addr);

void i2c_lld_set_evt_handlers(i2cStartHandler, i2cStopHandler);
void i2c_lld_set_slave_handlers(i2cRxHandler, i2cTxHandler);
void i2c_lld_set_master_handlers(i2cRxHandler, i2cTxHandler);

/** Configure the I2C hardware.
 * @param freq the clock frequency in kHz of the I2C master.
 */
void i2c_lld_set_freq(uint16_t freq);

/** Get the I2C master frequency.
 * @return the I2C master frequency in kHz.
 */
uint16_t i2c_lld_get_freq(void);

/** Set self slave address
 */
void i2c_lld_set_local(uint8_t addr);
uint8_t i2c_lld_get_local(void);

/** Reset I2C controller and bus
 */
void i2c_lld_clearbus(void);

uint8_t i2c_lld_start_transmission(uint8_t);
uint8_t i2c_lld_request(uint8_t);

#ifdef I2C_NO_ISR
#define I2C_FLAG_SET() (TWCR & _BV(TWINT))
void i2c_lld_loop(void);
#endif

#endif

