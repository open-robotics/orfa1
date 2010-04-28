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

//#include <avr/pgmspace.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "i2c.h"
#include "command.h"
#include "lib/hex.h"

#ifndef PROTOCOL_VERSION_STRING
# ifdef SG_PROTOCOL_V1_0
#  define PROTOCOL_VERSION_STRING  "1.0"
# else
#  define PROTOCOL_VERSION_STRING  "1.1"
# endif
#endif


void print_error(error_code_t error_code)
{
	putchar('E');
	putchar('R');
	putchar('R');
	putchar('O');
	putchar('R');
	putchar(' ');
	putchar(itox(error_code >> 4));
	putchar(itox(error_code & 0x0f));
	putchar('\n');
}

/*
 * Execute a cmd_buf sequence.
 *
 * \param[in]  cmd_buf buffer conraining the output command string.
 * \param[out] tx_buf buffer contining the messages sent to the user.
 * \return true on success, false on error.
 */
bool exec_cmd(cbf_t *cmd_buf, cbf_t *tx_buf, error_code_t *error_code)
{
	uint8_t c;
	uint8_t nbytes;
	uint16_t freq;
	uint8_t addr=0xaa;

	if (cbf_isempty(cmd_buf)) {
		return true;
	}
	c = cbf_get(cmd_buf);

	switch (c) {
		case 'V':
			cbf_put(tx_buf, c);
			for (uint8_t i = 0; i < sizeof(PROTOCOL_VERSION_STRING) - 1; i++) {
				cbf_put(tx_buf, PROTOCOL_VERSION_STRING[i]);
			}
			break;

		case 'C':			
			if (!cbf_isempty(cmd_buf)) {
				freq = (cbf_get(cmd_buf) << 8) | cbf_get(cmd_buf);
				if ((freq > 0) && (freq <= 800)) {
					i2c_config(freq);
				}
			}
			freq = i2c_get_freq();

			cbf_put(tx_buf, 'C');
			cbf_put(tx_buf, itox((freq >> 12) & 0x0f));
			cbf_put(tx_buf, itox((freq >>  8) & 0x0f));
			cbf_put(tx_buf, itox((freq >>  4) & 0x0f));
			cbf_put(tx_buf, itox((freq >>  0) & 0x0f));
			break;

		case 'L':
			if (!cbf_isempty(cmd_buf)) {
				addr = cbf_get(cmd_buf) & (~0x01);
				i2c_set_localhost(addr);
			}
			addr = i2c_get_localhost();

			cbf_put(tx_buf, 'L');
			cbf_put(tx_buf, itox((addr >>  4) & 0x0f));
			cbf_put(tx_buf, itox((addr >>  0) & 0x0f));
			break;

		case 'S':
			// I2C Read/Write command
			do {
				cbf_put(tx_buf, 'S');
				c = cbf_get(cmd_buf);
				if (c & 0x01) {
					// Read
					if (!i2c_master_start(c, i2c_rd)) {
						i2c_master_stop();
						debug("# ERROR: Nack on rd address\n");
						*error_code = NACK_ADDRESS;
						return false;
					}
					cbf_put(tx_buf, 'R');

					nbytes = cbf_get(cmd_buf);
					while (nbytes > 0) {
						if (!i2c_master_rxc(&c, nbytes > 1)) {
							i2c_master_stop();
							debug("# ERROR: Nack on rd byte\n");
							*error_code = NACK_BYTE;
							return false;
						}

						cbf_put(tx_buf, itox(c >> 4));
						cbf_put(tx_buf, itox(c & 0x0f));
						nbytes--;
					}
					c = cbf_get(cmd_buf);

				} else {
					// Write
					if (!i2c_master_start(c, i2c_wr)) {
						i2c_master_stop();
						debug("# ERROR: Nack on wr address\n");
						*error_code = NACK_ADDRESS;
						return false;
					}
					cbf_put(tx_buf, 'W');

					c = cbf_get(cmd_buf);
					while (c != 'S' && c != 'P') {
						nbytes = c;
						while (nbytes > 0) {
							c = cbf_get(cmd_buf);
							if (i2c_master_txc(c)) {
								cbf_put(tx_buf, 'A');
							} else {
								i2c_master_stop();
								debug("# ERROR: Nack on wr byte\n");
								*error_code = NACK_BYTE;
								return false;
							}
							nbytes--;
						}
						c = cbf_get(cmd_buf);
					}
				}
			} while (c == 'S');

			i2c_master_stop();

			if (c != 'P') {
				debug("# ERROR: P expected\n");
				*error_code = P_EXPECTED;
				return false;
			}

			cbf_put(tx_buf, 'P');
			break;

		case 'X':
			i2c_bus_clear();
			cbf_put(tx_buf, 'X');
			break;

		default:
			debug("# exec_cmd() internal error\n");
			return false;
			break;
	}

	// not needed: serial_fdev translate '\n' to '\r\n'
	// cbf_put(tx_buf, '\r');

	cbf_put(tx_buf, '\n');

	return true;
}

