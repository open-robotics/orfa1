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
/** RoboBus SPI driver
 * @file spi_driver.h
 *
 * @author Andrey Demenev
 * @author Vladimir Ermakov <vooon341@gmail.com>
 */

#ifndef SPI_DRIVER_H
#define SPI_DRIVER_H

#include "core/common.h"

#ifdef OR_AVR_M32_D
	#define GATE_SPI_PORT_NUMBER 1
	#define GATE_SPI_PORT PORTB
	#define GATE_SPI_DDR DDRB
	#define GATE_SPI_SS 4
	#define GATE_SPI_SCK 7
	#define GATE_SPI_MOSI 5
	#define GATE_SPI_MISO 6
#endif

#ifdef OR_AVR_M64_S
	#define GATE_SPI_PORT_NUMBER -1
	#define GATE_SPI_PORT PORTB
	#define GATE_SPI_DDR DDRB
	#define GATE_SPI_SS 0
	#define GATE_SPI_SCK 1
	#define GATE_SPI_MOSI 2
	#define GATE_SPI_MISO 3
#endif

#endif
