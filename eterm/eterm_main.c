/*
 *  ORFA -- Open Robotics Firmware Architecture
 *
 *  Copyright (c) 2010 Vladimir Ermakov
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
/** eTerm connector
 *
 * @author Ermakov Vladimir
 */

#include "eterm.h"
#include "eterm_main.h"
#include "hal/i2c.h"
#include "hal/serial.h"


void register_serialgate(void);
void register_orc32(void);
void register_port(void);
#ifdef HAVE_MOTOR
void register_md2(void);
#endif

void eterm_init(void) {
	register_serialgate();
	register_orc32();
	register_port();
	register_help();

	#ifdef HAVE_MOTOR
		register_md2();
	#endif

	#ifdef HAL_HAVE_SERIAL_FILE_DEVICE
	serial_init(BAUD);
	stdin = stdout = stderr = &serial_fdev;
	#endif

	i2c_init();
}

void eterm_supertask(void) {
	if(serial_isempty())
		return;

	uint8_t c = getchar();
	
	parse_command(c, false);
}

