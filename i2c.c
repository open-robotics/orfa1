/*
 *  userial -- a USB to I2C converter
 *
 *  Copyright (c) 2008-2009, Thomas Pircher <tehpeh@gmx.net>
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

#include "i2c.h"
#include "debug.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#define TWI_FREQ    100 // 100kHz
#define LOCALHOST_ADDR   0xAA

#ifdef AVR_IO

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/twi.h>

//***************************************************************************
//  TWI control register values
//***************************************************************************
#define TWICR_TWEN          (_BV(TWEN))                 //!< TWI enable
#define TWICR_TWIE          (_BV(TWIE))                 //!< interrupts enabled
#define TWICR_TWEA          (_BV(TWEA))                 //!< next I2C activity will be acknowledged
#define TWICR_TWSTA         (_BV(TWSTA))                //!< set the start bit
#define TWICR_TWSTO         (_BV(TWSTO))                //!< set the stop bit
#define TWICR_TWINT         (_BV(TWINT))                //!< clear onlythe TWINT bit
//#define TWICR_TWINT_TWEA    (_BV(TWINT)|_BV(TWEA))      //!< TWCR_TWINT and TWCR_TWEA


/*!
 * Get the TWPS (prescaler) bits for the TWI
 *
 * \param freq I2C master frequency in kHz.
 */
static inline uint8_t i2c_get_twps(uint16_t freq) __attribute__((const))
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

/*!
 * Calculates the TWBR (baudrate) bits for the TWI
 *
 * \param freq I2C master frequency in kHz.
 * \param twps the selected prescaler bits.
 */
static inline uint8_t i2c_get_twbr(uint16_t freq, uint8_t twps) __attribute__((const))
{
    return (((F_CPU / 2000) / freq) - 8) / (1 << (twps *2));
}


static inline void avr_i2c_init(void)
{
    TWCR = 0;
    i2c_config(TWI_FREQ);
    TWDR = 0xFF;
    TWCR = TWICR_TWEN;
}

static inline void avr_i2c_config(uint16_t freq)
{
    uint8_t twps;

    twps = i2c_get_twps(freq);
    TWBR = i2c_get_twbr(freq, twps);
    TWSR = twps & 0x03;
}

static inline uint16_t avr_i2c_get_freq(void)
{
    uint8_t twps = 1 << (2 * (TWSR & 0x03));
    uint16_t freq = F_CPU / 1000UL;
    return freq / (16 + 2*TWBR * twps);
}

static inline bool avr_i2c_master_start(uint8_t address, i2c_rdwr_t flag)
{
    bool error = false;
    uint8_t twsr;

    TWCR = TWICR_TWEN | TWICR_TWINT | TWICR_TWSTA;
    while ((TWCR & TWICR_TWINT) == 0);

    twsr = TWSR & TW_STATUS_MASK;
    if ((twsr != TW_START) && (twsr != TW_REP_START)) {
        error = true;
    } else {
        TWDR = (address & 0xfe) | flag;
        TWCR = TWICR_TWEN | TWICR_TWINT;
        while ((TWCR & TWICR_TWINT) == 0);

        twsr = TWSR & TW_STATUS_MASK;
        if (flag == i2c_rd) {
            error = twsr != TW_MR_SLA_ACK;
        } else {
            error = twsr != TW_MT_SLA_ACK;
        }
    }
    return !error;
}

static inline void avr_i2c_master_stop(void)
{
    TWCR = TWICR_TWEN | TWICR_TWINT | TWICR_TWSTO;
}

static inline bool avr_i2c_master_txc(uint8_t c)
{
    bool error = false;

    TWDR = c;
    TWCR = TWICR_TWEN | TWICR_TWINT;
    while ((TWCR & TWICR_TWINT) == 0);
    error = (TWSR & TW_STATUS_MASK) != TW_MT_DATA_ACK;

    return !error;
}

static inline bool avr_i2c_master_rxc(uint8_t *c, bool ack)
{
    bool error = false;
    uint8_t twsr;

    if (ack) {
        TWCR = TWICR_TWEN | TWICR_TWINT | TWICR_TWEA;
    } else {
        TWCR = TWICR_TWEN | TWICR_TWINT;
    }
    while ((TWCR & TWICR_TWINT) == 0);

    *c = TWDR;

    twsr = TWSR & TW_STATUS_MASK;
    if (ack) {
        error = twsr != TW_MR_DATA_ACK;
    } else {
        error = twsr != TW_MR_DATA_NACK;
    }

    return !error;
}

#endif // AVR_IO

static uint8_t localhost_addr = 0xaa;
static uint8_t current_addr = 0x00;
static bool is_started = false;
static i2c_localhost_start local_start=NULL;
static i2c_localhost_stop local_stop=NULL;
static i2c_localhost_txc local_txc=NULL;
static i2c_localhost_rxc local_rxc=NULL;

void i2c_init()
{
    #ifdef AVR_IO
    avr_i2c_init();
    #endif
    debug("# i2c_init()\n");

    localhost_addr = LOCALHOST_ADDR;
}

void i2c_bus_clear()
{
    i2c_master_start(0, i2c_rd);
    i2c_master_stop();
}

void i2c_config(uint16_t freq)
{
    #ifdef AVR_IO
    avr_i2c_config(freq);
    #endif
    debug("# i2c_config(%i)\n", freq);
}

uint16_t i2c_get_freq()
{
    debug("# i2c_get_freq()\n");
    #ifdef AVR_IO
    return avr_i2c_get_freq();
    #else
    return TWI_FREQ;
    #endif
}

void i2c_set_handlers(i2c_localhost_start start, i2c_localhost_stop stop, i2c_localhost_txc txc, i2c_localhost_rxc rxc)
{
    local_start = start;
    local_stop = stop;
    local_txc = txc;
    local_rxc = rxc;
}

void i2c_set_localhost(uint8_t address)
{
    debug("# i2c_set_localhost(0x%02x)\n", address);
    localhost_addr = address & 0xfe;
}

uint8_t i2c_get_localhost(void)
{
    return localhost_addr;
}

bool i2c_master_start(uint8_t address, i2c_rdwr_t flag)
{
    bool status=false;

    current_addr = address & 0xfe;
    is_started = true;

    if(current_addr == localhost_addr)
    {
        debug("# i2c_master_start(0x%02x, %i) -> localhost\n", address, flag);
        if(local_start != NULL)
        {
            status = local_start(address, flag);
        }
        else
        {
            status = false;
        }
    }
    else
    {
        #ifdef AVR_IO
        status = avr_i2c_master_start(address, flag);
        #else
        status = true;
        #endif
        debug("# i2c_master_start(0x%02x, %i) -> real\n", address, flag);
    }

    return status;
}

void i2c_master_stop()
{
    is_started = false;
    #ifdef AVR_IO
    avr_i2c_master_stop();
    #endif
    debug("# i2c_master_stop()\n");
    if(local_stop != NULL)
    {
        local_stop();
    }
}

bool i2c_master_txc(uint8_t c)
{
    bool status=false;

    if(current_addr != localhost_addr)
    {
        #ifdef AVR_IO
        status = avr_i2c_master_txc(c);
        #else
        status = true;
        #endif
        debug("# i2c_master_txc(0x%02x) -> real\n", c);
    }
    else if(is_started)
    {
        debug("# i2c_master_txc(0x%02x) -> localhost\n", c);
        if(local_txc != NULL)
        {
            status = local_txc(c);
        }
    }
    else
    {
        debug("# ERROR: i2c_master_txc(0x%02x) not STARTED\n", c);
    }

    return status;
}

bool i2c_master_rxc(uint8_t *c, bool ack)
{
    bool status=false;

    if(current_addr != localhost_addr)
    {
        #ifdef AVR_IO
        status = avr_i2c_master_rxc(c, ack);
        #else
        status = true;
        *c = 0xff;
        #endif
        debug("# i2c_master_rxc(0x%02x, %i) -> real\n", *c, ack);
    }
    else if(is_started)
    {
        debug("# i2c_master_rxc(0x%02x, %i) -> localhost\n", *c, ack);
        if(local_rxc != NULL)
        {
            status = local_rxc(c, ack);
        }
    }
    else
    {
        debug("# ERROR: i2c_master_rxc(0x%02x, %i) not STARTED\n", *c, ack);
    }

    return status;
}
