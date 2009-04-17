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

//#include <avr/pgmspace.h>
#include <ctype.h>

//#include "i2c.h"
#include "cbuf.h"
#include "serial-command.h"

/*!
 * Convert a 4-bit integer value to its ASCII representation.
 *
 * \param c the value to convert.
 * \return the converted value or 'X' on error.
 */
static char itox(uint8_t c)
{
    char ret;

    if (c <= 0x09) {
        ret = '0' + c;
    } else if (c <= 0x0f) {
        ret = 'A' + c - 10;
    } else {
        ret = 'X';
    }
    return ret;
}


/*!
 * Execute a cmd_buf sequence.
 *
 * \param[in]  cmd_buf buffer conraining the output command string.
 * \param[out] tx_buf buffer contining the messages sent to the user.
 * \return true on success, false on error.
 */
bool exec_cmd(cbf_t *cmd_buf, cbf_t *tx_buf)
{
    uint8_t c;
    uint8_t nbytes;

    if (cbf_isempty(cmd_buf)) {
        return true;
    }
    c = cbf_get(cmd_buf);

    if (c == 'V') {
        cbf_put(tx_buf, c);
        for (uint8_t i = 0; i < sizeof(VERSION_STRING) - 1; i++) {
            cbf_put(tx_buf, VERSION_STRING[i]);
        }
    } else if (c == 'C') {
        c = cbf_get(cmd_buf);
        uint16_t freq;

        if (!cbf_isempty(cmd_buf)) {
            freq = (cbf_get(cmd_buf) << 8) | cbf_get(cmd_buf);
            if ((freq > 0) && (freq <= 800)) {
                //i2c_config(freq);
                printf("# I2C config freq: %i\n", freq);
            }
        }
        //freq = i2c_get_freq();

        cbf_put(tx_buf, 'C');
        cbf_put(tx_buf, itox((freq >> 12) & 0x0f));
        cbf_put(tx_buf, itox((freq >>  8) & 0x0f));
        cbf_put(tx_buf, itox((freq >>  4) & 0x0f));
        cbf_put(tx_buf, itox((freq >>  0) & 0x0f));

    } else if (c == 'S') {
        // I2C Read/Write command
        do {
            cbf_put(tx_buf, 'S');
            c = cbf_get(cmd_buf);
            if ((c & 0x01) == 0x01) {
                // Read
                /*if (!i2c_master_start(c, i2c_rd)) {
                    i2c_master_stop();
                    return parse_error(tx_buf, 'N', PSTR("Nack on rd address"));
                }*/
                printf("# I2C read address: 0x%02X\n", c);

                cbf_put(tx_buf, 'A');

                nbytes = cbf_get(cmd_buf);
                while (nbytes > 0) {
                    /*if (!i2c_master_rx(&c, 1, nbytes > 1)) {
                        i2c_master_stop();
                        return parse_error(tx_buf, 'N', PSTR("Nack on rd byte"));
                    }*/
                    c = 0xff;
                    printf("# I2C read byte %i: 0x%02X\n", nbytes, c);

                    cbf_put(tx_buf, itox(c >> 4));
                    cbf_put(tx_buf, itox(c & 0x0f));
                    nbytes--;
                }
                c = cbf_get(cmd_buf);
            } else {
                // Write
                /*if (!i2c_master_start(c, i2c_wr)) {
                    i2c_master_stop();
                    return parse_error(tx_buf, 'N', PSTR("Nack on wr address"));
                }*/
                printf("# I2C write address: 0x%02X\n", c);

                cbf_put(tx_buf, 'A');

                c = cbf_get(cmd_buf);
                while (c != 'S' && c != 'P') {
                    nbytes = c;
                    while (nbytes > 0) {
                        c = cbf_get(cmd_buf);
                        printf("# I2C write byte: 0x%02X\n", c);
                        //if (i2c_master_tx(&c, 1)) {
                            cbf_put(tx_buf, 'A');
                        /*} else {
                            i2c_master_stop();
                            return parse_error(tx_buf, 'N', PSTR("Nack on wr byte"));
                        }*/
                        nbytes--;
                    }
                    c = cbf_get(cmd_buf);
                }
            }
        } while (c == 'S');

        if (c != 'P') {
            //i2c_master_stop();
            //return parse_error(tx_buf, 'F', PSTR("P expected"));
            printf("# I2C ERROR: P expected\n");
            return false;
        }

        //i2c_master_stop();
        cbf_put(tx_buf, 'P');

    } else if (c == 'X') {
        //i2c_bus_clear();
        printf("# I2C clear\n");
        cbf_put(tx_buf, 'X');
    } else {
        //return parse_error(tx_buf, 'F', PSTR("C or S expected after I"));
        printf("# I2C Internal error\n");
        return false;
    }

    cbf_put(tx_buf, '\r');
    cbf_put(tx_buf, '\n');

    return true;
}
