#ifndef I2C_H
#define I2C_H

#include <stdint.h>
#include <stdbool.h>
	
#define I2C_E_OK		0
#define I2C_E_ARB		1
#define I2C_E_ADDR_NACK	2
#define I2C_E_DATA_NACK	3
#define I2C_E_BUS		4
#define I2C_E_BUFSIZE	5

typedef bool (*i2cRxHandler)(uint8_t);
typedef bool (*i2cTxHandler)(uint8_t*, bool*);

uint8_t i2c_startTransmission(uint8_t);

typedef bool (*i2cStartHandler)(uint8_t flag);
typedef void (*i2cStopHandler)(void);

void i2c_init(void);
void i2c_init_slave(uint8_t addr);
void i2c_onStart(i2cStartHandler callback);
void i2c_onStop(i2cStopHandler callback);
void i2c_onRX(i2cRxHandler callback);
void i2c_onRequest(i2cTxHandler callback);


uint8_t i2c_receive(void);
bool i2c_send(uint8_t data);

#ifdef I2C_NO_ISR
#define I2C_FLAG_SET() (TWCR & _BV(TWINT))
void i2c_loop(void);
#endif

	
void i2c_set_handlers(i2cStartHandler, i2cStopHandler, i2cRxHandler, i2cTxHandler);
void i2c_gate_supertask(void);
#define gate_supertask i2c_gate_supertask
void i2c_gate_init(void);
#define gate_init i2c_gate_init

#endif

