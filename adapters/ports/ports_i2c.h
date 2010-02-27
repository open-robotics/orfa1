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
/** RoboGPIO Digital I2C adapter
 * @file ports_i2c.h
 *
 * @author Andrey Demenev
 * @author Vladimir Ermakov <vooon341@gmail.com>
 */

#ifndef PORTS_DRIVER_H
#define PORTS_DRIVER_H

#include "core/common.h"

#ifdef OR_AVR_M32_D
	#define GATE_NUM_PORTS 4
	#define GATE_PORT_MASK_A 0
	#define	GATE_PORT_MASK_B 0xE3
	#define	GATE_PORT_MASK_C 0x03
	#define	GATE_PORT_MASK_D 0x33
	#define GATE_PORT_SPECS GATE_PORT_SPEC(A), GATE_PORT_SPEC(B), GATE_PORT_SPEC(C), GATE_PORT_SPEC(D)
	#define GATE_PORT_UID 0x0020
#endif

#ifdef OR_AVR_M128_S
	#define GATE_NUM_PORTS 4
	#define GATE_PORT_MASK_A 0
	#define	GATE_PORT_MASK_B 0xFE
	#define GATE_PORT_MASK_E 0x3F
	#define	GATE_PORT_MASK_F 0
	#define GATE_PORT_SPECS GATE_PORT_SPEC(A), GATE_PORT_SPEC(F), GATE_PORT_SPEC(B), GATE_PORT_SPEC(E)
	#define GATE_PORT_UID 0x0021
#endif

#ifdef OR_AVR_M128_DS
	#define GATE_NUM_PORTS 4
	#define GATE_PORT_MASK_A 0
	#define	GATE_PORT_MASK_B 0xFE
	#define GATE_PORT_MASK_E 0x3F
	#define	GATE_PORT_MASK_F 0
	#define GATE_PORT_SPECS GATE_PORT_SPEC(A), GATE_PORT_SPEC(F), GATE_PORT_SPEC(B), GATE_PORT_SPEC(E)
	#define GATE_PORT_UID 0x0021
#endif

#define GATE_PORT_SPEC(p) \
	{\
		.PORT = (void*)_SFR_MEM_ADDR(PORT ## p),				\
		.PIN = (void*)_SFR_MEM_ADDR(PIN ## p),					\
		.DDR = (void*)_SFR_MEM_ADDR(DDR ## p ),					\
		.default_busy_mask = GATE_PORT_MASK_ ## p,				\
	}

#endif

