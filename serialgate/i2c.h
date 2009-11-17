/*
 *  ORFA -- Open Robotics Firmware Architecture
 *  Based on userial -- a USB to I2C converter
 *
 *  Copyright (c) 2008-2009, Thomas Pircher <tehpeh@gmx.net>
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
/** I2C Master lib
 * @file i2c.h
 *
 * @author Thomas Pircher <tehpeh@gmx.net>
 * @author Vladimir Ermakov <vooon341@gmail.com>
 */

#ifndef I2C_H
#define I2C_H

#include <stdbool.h>
#include <stdint.h>

/**
 * I2C read or write status flags
 */
typedef enum {
	i2c_wr = 0,	 ///< the following operation is a write operation.
	i2c_rd = 1,	 ///< the following operation is a read operation.
} i2c_rdwr_t;

/// Localhost start request processors
typedef bool (*i2c_localhost_start)(i2c_rdwr_t flag);

/// Localhost stop request processors
typedef void (*i2c_localhost_stop)(void);

/// Localhost transmit request processors
typedef bool (*i2c_localhost_txc)(uint8_t c);

/// Localhost receive request processors
typedef bool (*i2c_localhost_rxc)(uint8_t *c, bool *ack);

/**
 * Initialise the I2C hardware.
 */
void i2c_init(void);

/**
 * Clear the I2C bus.
 */
void i2c_bus_clear(void);

/**
 * Configure the I2C hardware.
 *
 * @param freq the clock frequency in kHz of the I2C master.
 */
void i2c_config(uint16_t freq);

/**
 * Get the I2C master frequency.
 *
 * @return the I2C master frequency in kHz.
 */
uint16_t i2c_get_freq(void);

/**
 * Set localhost i2c handlers
 */
void i2c_set_handlers(i2c_localhost_start start, i2c_localhost_stop stop, i2c_localhost_txc txc, i2c_localhost_rxc rxc);

/**
 * Configure the localhost address
 */
void i2c_set_localhost(uint8_t address);

/**
 * Configure the localhost address
 */
uint8_t i2c_get_localhost(void);

/**
 * Put a Start Condition on the bus.
 *
 * @param address the I2C slave address (left aligned).
 * @param flag indicates if the following operation is a read or write operation.
 * @return true on success, false on error.
 */
bool i2c_master_start(uint8_t address, i2c_rdwr_t flag);

/**
 * Put a Stop Condition on the bus.
 */
void i2c_master_stop(void);

/**
 * Write to a I2C slave.
 *
 * @param buf the buffer with the data to write.
 * @param buf_len the number of bytes to write.
 * @return true on success, false on error.
 */
bool i2c_master_txc(uint8_t c);

/**
 * Read from a I2C slave.
 *
 * @param buf the buffer that store the read bytes.
 * @param buf_len the number of bytes in the buffer.
 * @param ack if true, send an ack (transmission contines) after the last byte
 *			otherwise send a nack (transmission ends).
 * @return true on success, false on error.
 */
bool i2c_master_rxc(uint8_t *c, bool ack);

#endif // !defined I2C_H
