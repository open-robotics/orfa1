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
/** ORFA I2C master/slave
 * @file i2c.c
 *
 * @author Andrey Demenev
 * @author Vladimir Ermakov
 * @author Thomas Pircher <tehpeh@gmx.net>
 */

// vim: set foldmethod=marker :

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/twi.h>
#include <stdint.h>
#include "i2c_lld.h"


#define I2C_IDLE	0
#define I2C_MTX		1
#define I2C_MRX		2
#define I2C_STX		3
#define I2C_SRX		4
#define I2C_MSTART  4

#ifndef I2C_NO_ISR
#define TWCR_TWIE_IF_ISR  _BV(TWIE)
#else
#define TWCR_TWIE_IF_ISR  0
#endif


#ifdef I2C_MASTER
static uint8_t slarw;
static uint8_t error = I2C_E_OK;
static volatile uint8_t state = I2C_IDLE;
static i2cTxHandler masterTxHandler;
static i2cRxHandler masterRxHandler;
#endif

static i2cStartHandler startHandler = NULL;
static i2cStopHandler stopHandler = NULL;

#ifdef I2C_SLAVE
static uint8_t slave_addr = 0;
static i2cRxHandler slaveRxHandler = NULL;
static i2cTxHandler slaveTxHandler = NULL;
static volatile uint8_t inCallback = 0;
#endif


void i2c_lld_set_evt_handlers(
		i2cStartHandler start,
		i2cStartHandler stop)
{
	startHandler = start;
	stopHandler = stop;
}

void i2c_lld_set_slave_handlers(
		i2cRxHandler slave_rx,
		i2cTxHandler slave_tx)
{
	slaveRxHandler = slave_rx;
	slaveTxHandler = slave_tx;
}

void i2c_lld_set_master_handlers(
		i2cRxHandler master_rx,
		i2cTxHandler master_tx)
{
	masterRxHandler = master_rx;
	masterTxHandler = master_tx;
}


static void reply(uint8_t ack)
{
	if (ack) {
		TWCR =
				_BV(TWEN)
			|	TWCR_TWIE_IF_ISR
            |	_BV(TWINT)
			|	_BV(TWEA)
			;
	} else {
		TWCR =
				_BV(TWEN)
			|	TWCR_TWIE_IF_ISR
            |	_BV(TWINT)
            ;
	}
}

#ifdef I2C_MASTER
static void send_stop(void)
{
	TWCR = 
			_BV(TWEN)
		|	TWCR_TWIE_IF_ISR
        |	_BV(TWEA)
		|	_BV(TWINT)
		|	_BV(TWSTO)
		;
	
	//while(TWCR & _BV(TWSTO));
	state = I2C_IDLE;
}
#endif

static void release(void)
{
	TWCR =
			_BV(TWEN)
		|	TWCR_TWIE_IF_ISR
        |	_BV(TWEA)
		|	_BV(TWINT)
		;

#ifdef I2C_MASTER
	state = I2C_IDLE;
#endif
}


#ifdef I2C_NO_ISR
void i2c_lld_loop(void);
#else
ISR(SIG_2WIRE_SERIAL)
#endif
{
	uint8_t status = TWSR & 0xF8;
    bool ack = 1;

	switch (status) {
#ifdef I2C_SLAVE
		// slave receiver
		/*{{{*/
		case TW_SR_ARB_LOST_SLA_ACK:
		case TW_SR_SLA_ACK:
#  ifdef I2C_MASTER
			state = I2C_SRX;
#  endif
            if (startHandler) {
				reply(startHandler(0));
            } else {
                reply(1);
            }

#  ifdef I2C_MASTER
			if (status == TW_SR_ARB_LOST_SLA_ACK) {
				error = I2C_E_ARB;
			}
#  endif
			break;
			
		case TW_SR_DATA_ACK:
            if (slaveRxHandler) {
                reply(slaveRxHandler(TWDR));
            } else {
                reply(1);
            }
			break;

		case TW_SR_DATA_NACK:
			reply(1);
			break;

		case TW_SR_STOP:
            if (stopHandler) {
                stopHandler();
            }
#  ifdef I2C_MASTER
			state = I2C_IDLE;
#  endif
			reply(1);
			break;

		/*}}}*/

		// slave transmitter
		/*{{{*/
		case TW_ST_SLA_ACK:
		case TW_ST_ARB_LOST_SLA_ACK:
#  ifdef I2C_MASTER
			state = I2C_STX;
			if (status == TW_ST_ARB_LOST_SLA_ACK) {
				error = I2C_E_ARB;
			}
#  endif
			startHandler(1);
			// fallback
		case TW_ST_DATA_ACK:
            if (slaveTxHandler) {
                uint8_t c = 0;
                slaveTxHandler(&c, &ack);
                TWDR = c;
                reply(ack);
            } else {
                TWDR = 0;
                reply(0);
            }
			break;

		case TW_ST_LAST_DATA:
		case TW_ST_DATA_NACK:
			reply(1);

#  ifdef I2C_MASTER
			state = I2C_IDLE;
#  endif
			break;
		/*}}}*/
#endif // I2C_SLAVE

#ifdef I2C_MASTER
		// master transmitter
		/*{{{*/
		case TW_MT_SLA_ACK:
		case TW_MT_DATA_ACK:
            if (masterTxHandler) {
                uint8_t c;
                ack = masterTxHandler(&c, &ack);
                if (ack) {
    				TWDR = c;
				    reply(1);
                    state = I2C_MTX;
                } else {
				    send_stop();
                }
			} else {
				send_stop();
			}
			break;

		case TW_MT_SLA_NACK:
			error = I2C_E_ADDR_NACK;
			send_stop();
			break;
		case TW_MT_DATA_NACK:
			error = I2C_E_DATA_NACK;
			send_stop();
			break;

		case TW_MT_ARB_LOST:
			error = I2C_E_ARB;
			release();
			break;
		/*}}}*/

		//  master receiver
		/*{{{*/
		case TW_MR_DATA_ACK:
            if (masterRxHandler) {
                ack = masterRxHandler(TWDR);
            }
			// fallback
		case TW_MR_SLA_ACK:
			reply(ack);
			break;
		case TW_MR_DATA_NACK:
            if (masterRxHandler) {
                masterRxHandler(TWDR);
            }
			// fallback
		case TW_MR_SLA_NACK:
			send_stop();
			break;
		/*}}}*/

		// master
		/*{{{*/
		case TW_START:
		case TW_REP_START:
			TWDR = slarw;
			reply(1);
			break;
		/*}}}*/
#endif // I2C_MASTER

		//  common
		/*{{{*/
		case TW_NO_INFO:
			break;
		case TW_BUS_ERROR:
#ifdef I2C_MASTER
		  error = I2C_E_BUS;
		  send_stop();
#else
		  release();
#endif
		  break;

		/*}}}*/

	}
}


#ifdef I2C_MASTER
uint8_t i2c_lld_start_transmission(uint8_t addr)
{
	while (state != I2C_IDLE) {
#  ifdef I2C_NO_ISR
        i2c_lld_loop();
#  endif
    };

	error = I2C_E_OK;
	slarw = TW_WRITE | (addr << 1);
    state = I2C_MSTART;
	TWCR = 
			_BV(TWINT)
		|	_BV(TWSTA)
		|	_BV(TWEN)
		|	TWCR_TWIE_IF_ISR
		;

	while (state != I2C_IDLE) {
#  ifdef I2C_NO_ISR
        i2c_lld_loop();
#  endif
    };
    return error;
}


uint8_t i2c_lld_request(uint8_t addr)
{
	slarw = TW_READ | (addr << 1);

	state = I2C_MRX;
	error = I2C_E_OK;
	TWCR = 
			_BV(TWINT)
		|	_BV(TWSTA)
		|	_BV(TWEN)
		|	TWCR_TWIE_IF_ISR
        ;
	while (state == I2C_MRX);

	return error;
}
#endif // I2C_MASTER


/** Get the TWPS (prescaler) bits for the TWI
 * @param freq I2C master frequency in kHz.
 */
static inline uint8_t i2c_get_twps(uint16_t freq) __attribute__((const));
static inline uint8_t i2c_get_twps(uint16_t freq)
{
	uint8_t ps;
	uint16_t val = F_CPU / (0xffUL * 2000UL);

	ps = 0;
	while (freq <= val) {
		ps++;
		val /= 4;
	}
	return ps;
}

/** Calculates the TWBR (baudrate) bits for the TWI
 * @param freq I2C master frequency in kHz.
 * @param twps the selected prescaler bits.
 */
static inline uint8_t i2c_get_twbr(uint16_t freq, uint8_t twps) __attribute__((const));
static inline uint8_t i2c_get_twbr(uint16_t freq, uint8_t twps)
{
	return (((F_CPU / 2000) / freq) - 8) / (1 << (twps *2));
}

static void i2c_lld_set_freq(uint16_t freq)
{
	uint8_t twps;

	twps = i2c_get_twps(freq);
	TWBR = i2c_get_twbr(freq, twps);
	TWSR = twps & 0x03;
}

static uint16_t i2c_lld_get_freq(void)
{
	uint8_t twps = 1 << (2 * (TWSR & 0x03));
	uint16_t freq = F_CPU / 1000UL;
	return freq / (16 + 2*TWBR * twps);
}

void i2c_lld_init(void)
{
	//TWBR = 7;
	//TWSR = 1;
	i2c_lld_set_freq(100);
#ifdef I2C_SLAVE
	TWAR = slave_addr << 1;
#else
	TWAR = 0;
#endif
	TWCR = 
			_BV(TWINT)
		|	_BV(TWEA)
		|	_BV(TWEN)
		|	TWCR_TWIE_IF_ISR
        ;

}

#ifdef I2C_SLAVE
void i2c_lld_init_slave(uint8_t addr)
{
	slave_addr = addr;
	i2c_init();
}

void i2c_lld_set_local(uint8_t addr)
{
	slave_addr = addr;
	TWAR = slave_addr << 1;
}

uint8_t i2c_lld_get_local(void)
{
	return slave_addr;
}
#endif

