/*
 *  ORFA -- Open Robotics Firmware Architecture
 *
 *  Copyright (c) 2009 Vladimir Ermakov, Andrey Demenev, Anton Botov
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
// vim: set noet:

#ifndef OR_AVR_M32_D
#error servoGPIO: unsupported platform
#endif

#include <avr/io.h>
#include <avr/interrupt.h>
#include "core/common.h"
#include "core/driver.h"
#include <stdint.h>
#include <stdbool.h>

#ifndef NDEBUG
#include "serialgate/common.h"
#endif

#define RESOLUTION_IN_TICKS 32
#define RESOLUTION_TIME     (F_CPU / RESOLUTION_IN_TICKS)
#define MAXSERVO            (F_CPU/ 400/RESOLUTION_IN_TICKS)
#define MINSERVO            (F_CPU/2000/RESOLUTION_IN_TICKS)
#define WORKSPACE           (MAXSERVO+1)

#define CODE_FOR_ENABLE(servo_id, ddr, pin) \
	if (gpio_servo_enb[servo_id]) { \
		ddr |= (1<<pin); \
	} else { \
		ddr &= ~(1<<pin); \
	}


static uint16_t gpio_servo_pos[16];
static bool gpio_servo_enb[16];

//static GATE_RESULT driver_read(uint8_t reg, uint8_t* data, uint8_t* data_len);
static GATE_RESULT driver_write(uint8_t reg, uint8_t* data, uint8_t data_len);

static GATE_DRIVER driver = {
	.uid = 0x0031,
	.major_version = 1,
	.minor_version = 0,
//	.read = driver_read,
	.write = driver_write,
	.num_registers = 2,
};

uint8_t pause0[40];
uint8_t pause1[40];
uint8_t mask0[40];
uint8_t mask1[40];
uint8_t iterator;

uint8_t port_mask[16] = {
	1<<0,
	1<<1,
	1<<2,
	1<<3,
	1<<4,
	1<<5,
	1<<6,
	1<<7,
	1<<7,
	1<<6,
	1<<5,
	1<<4,
	1<<3,
	1<<2,
	1<<5,
	1<<4
};

#define portHandlerOR(name,port,nextHandler,id) \
void name(void)\
{\
   port |= mask0[id]; \
   OCR2 += pause0[id]; \
   handler = nextHandler; \
}

#define portHandlerAND(name,port,nextHandler,id) \
void name(void)\
{\
   port &= mask0[id]; \
   OCR2 += pause0[id]; \
   for(iterator=0; iterator<pause1[id]; iterator++){
     delay_us(1);
   };
   port &= mask1[id]; \
   handler = nextHandler; \
}

#define portHandlers(process,port,nextProcess,id) \
portHandlerAND(##process##4,port,##nextProcess##0,(id+4))\
portHandlerAND(##process##3,port,##process##4,(id+3))\
portHandlerAND(##process##2,port,##process##3,(id+2))\
portHandlerAND(##process##1,port,##process##2,(id+1))\
portHandlerOR(##process##0,port,##process##1,(id))\

void processD0(void);

portHandlers(processAa, PORTA, processD, 0)
portHandlers(processAb, PORTA, processAa, 4)
portHandlers(processAc, PORTA, processAb, 8)
portHandlers(processAd, PORTA, processAc, 12)
portHandlers(processCa, PORTC, processAd, 16)
portHandlers(processCb, PORTC, processCa, 20)
portHandlers(processB, PORTB, processCb, 24)
portHandlers(processD, PORTD, processB, 28)

#define generateParams(port_id, param_id) \
uint8_t maskX0=0,maskX1=0; \
uint16_t pauseX0=0, pauseX1=0; \
if(gpio_servo_enb[port_id]){ maskX0=port_mask[port_id]; pauseX0=gpio_servo_pos[port_id]; }; \
if(gpio_servo_enb[port_id+1]){ maskX1=port_mask[port_id+1]; pauseX1=gpio_servo_pos[port_id+1]; }; \
if(pauseX0>pauseX1){ \
  uint8_t tmp8=pauseX0; pauseX0=pauseX1; pauseX1=tmp8; \
  uint16_t tmp16=maskX0; maskX0=maskX1; maskX1=tmp16; \
}; \
mask0[param_id]=maskX0|maskX1; \
if(pauseX1-pauseX0<2){ \
  if(pauseX0>400){ \
    pause0[0]=190; mask0[0]=maskX0|maskX1; \
	pause0[1]=190; mask0[1]=0xFF; pause1[1]=0; mask1[1]=0xFF; \
	pause0[2]=pauseX0-380; mask0[2]=0xFF; pause1[2]=0; mask1[2]=0xFF; \
	if(pauseX0==pauseX1){ \
	  pause0[3]=590-pauseX0; mask0[3]=~(maskX0|maskX1); pause1[3]=0; mask1[3]=0xFF; \
    }else{ \
      pause0[3]=590-pauseX0; mask0[3]=~maskX0; pause1[3]=pauseX1-pauseX0; mask1[3]=~maskX1; \
    }; \
	pause0[4]=10; mask0[4]=0xFF; pause1[4]=0; mask1[4]=0xFF; \
  }else if(pauseX0>200){ \
    pause0[0]=190; mask0[0]=maskX0|maskX1; \
	pause0[1]=pauseX0-190; mask0[1]=0xFF; pause1[1]=0; mask1[1]=0xFF; \
	if(pauseX0==pauseX1){ \
	  pause0[2]=90; mask0[2]=~(maskX0|maskX1); pause1[2]=0; mask1[2]=0xFF; \
    }else{ \
	  pause0[2]=90; mask0[2]=~maskX0; pause1[2]=pauseX1-pauseX0; mask1[2]=~maskX1; \
    };
	pause0[3]=420-pauseX0; mask0[3]=0xFF; pause1[3]=0; mask1[3]=0xFF; \
	pause0[4]=90; mask0[4]=0xFF; pause1[4]=0; mask1[4]=0xFF; \
  }else{ \
    pause0[0]=pauseX0; mask0[0]=maskX0|maskX1; \
	if(pauseX0==pauseX1){ \
      pause0[1]=250-pauseX0; mask0[1]=~(maskX0|maskX1); pause1[1]=0; mask1[1]=0xFF; \
	}else{
	  pause0[1]=250-pauseX0; mask0[1]=~maskX0; pause1[1]=pauseX1-pauseX0; mask1[1]=~maskX1; \
    };
	pause0[2]=100; mask0[2]=0xFF; pause1[2]=0; mask1[2]=0xFF; \
	pause0[3]=150; mask0[3]=0xFF; pause1[3]=0; mask1[3]=0xFF; \
	pause0[4]=100; mask0[4]=0xFF; pause1[4]=0; mask1[4]=0xFF; \
  }; \
}else{ \
  if(pauseX1-pauseX0>235){ \
	pause0[0]=110; mask0[0]=maskX0|maskX1; \
	pause0[1]=pauseX0-110; mask0[1]=0xFF; pause1[1]=0; mask1[1]=0xFF; \
	pause0[2]=230; mask0[2]=~maskX0; pause1[2]=0; mask1[2]=0xFF; \
	pause0[3]=pauseX1-pauseX0-230; mask0[3]=0xFF; pause1[3]=0; mask1[3]=0xFF; \
	pause0[4]=600-pauseX1; mask0[4]=~maskX1; pause1[4]=0; mask1[4]=0xFF; \
  }else{ \
    if(pauseX0>440){ \
      pause0[0]=210; mask0[0]=maskX0|maskX1; \
	  pause0[1]=210; mask0[1]=0xFF; pause1[1]=0; mask1[1]=0xFF; \
	  pause0[2]=pauseX0-420; mask0[2]=0xFF; pause1[2]=0; mask1[2]=0xFF; \
	  pause0[3]=pauseX1-pauseX0; mask0[3]=~maskX0; pause1[3]=0; mask1[3]=0xFF; \
	  pause0[4]=600-pauseX1; mask0[4]=~maskX1; pause1[4]=0; mask1[4]=0xFF; \
    }else if(pauseX0>220){ \
      pause0[0]=210; mask0[0]=maskX0|maskX1; \
	  pause0[1]=pauseX0-210; mask0[1]=0xFF; pause1[1]=0; mask1[1]=0xFF; \
	  pause0[2]=pauseX1-pauseX0; mask0[2]=~maskX0; pause1[2]=0; mask1[2]=0xFF; \
	  uint16_t paus=600-pauseX1;
	  pause0[3]=paus >> 1; mask0[3]=~maskX1; pause1[3]=0; mask1[3]=0xFF; \
	  pause0[4]=paus-(pause >> 1); mask0[4]=0xFF; pause1[4]=0; mask1[4]=0xFF; \
    }else{ \
      pause0[0]=10; mask0[0]=maskX0|maskX1; \
	  pause0[1]=pauseX0-10; mask0[1]=0xFF; pause1[1]=0; mask1[1]=0xFF; \
	  pause0[2]=pauseX1-pauseX0; mask0[2]=~maskX0; pause1[2]=0; mask1[2]=0xFF; \
	  uint16_t paus=600-pauseX1;
	  pause0[3]=paus >> 1; mask0[3]=~maskX1; pause1[3]=0; mask1[3]=0xFF; \
	  pause0[4]=paus-(pause >> 1); mask0[4]=0xFF; pause1[4]=0; mask1[4]=0xFF; \
    }; \
  }; \
};


ISR(SIG_OUTPUT_COMPARE2)
{
   handler();
}

void generateParameters(void)
{
  generateParams(0,0);
  generateParams(2,5);
  generateParams(4,10);
  generateParams(6,15);
  generateParams(8,20);
  generateParams(10,25);
  generateParams(12,30);
  generateParams(14,35);
};

static inline void set_enable(uint8_t n, bool enable)
{
	gpio_servo_enb[n] = (enable > 0) ? true : false;
	
	CODE_FOR_ENABLE(0,  DDRA, 0);
	CODE_FOR_ENABLE(1,  DDRA, 1);
	CODE_FOR_ENABLE(2,  DDRA, 2);
	CODE_FOR_ENABLE(3,  DDRA, 3);
	CODE_FOR_ENABLE(4,  DDRA, 4);
	CODE_FOR_ENABLE(5,  DDRA, 5);
	CODE_FOR_ENABLE(6,  DDRA, 6);
	CODE_FOR_ENABLE(7,  DDRA, 7);
	CODE_FOR_ENABLE(8,  DDRC, 7);
	CODE_FOR_ENABLE(9,  DDRC, 6);
	CODE_FOR_ENABLE(10, DDRC, 5);
	CODE_FOR_ENABLE(11, DDRC, 4);
	CODE_FOR_ENABLE(12, DDRB, 3);
	CODE_FOR_ENABLE(13, DDRB, 2);
	CODE_FOR_ENABLE(14, DDRD, 5);
	CODE_FOR_ENABLE(15, DDRD, 4);
}

static inline void set_position(uint8_t n, uint32_t pos)
{
	if (n > 15) 
		return;

	pos = pos * RESOLUTION_TIME/1000000;
	
	if (pos < MINSERVO)
		pos = MINSERVO;
	else if (pos > MAXSERVO)
		pos = MAXSERVO;

	gpio_servo_pos[n] = pos;
	generateParameters();
}


/* comment out now
static GATE_RESULT driver_read(uint8_t reg, uint8_t* data, uint8_t* data_len)
{
	*data_len = 0;
	return GR_OK;
}
*/

static GATE_RESULT driver_write(uint8_t reg, uint8_t* data, uint8_t data_len)
{
#ifndef NDEBUG
	debug("# servo_gpio->write(0x%02X, buf, %i)\n", reg, data_len);
#endif
	
	if (reg > 1) {
		return GR_NO_ACCESS;
	}

	if (reg == 0) {
		if (data_len != 3) {
			return GR_INVALID_DATA;
		}

		uint8_t byte = 0;
		while (byte < 2) {
			uint8_t bit = 0;
			while (bit < 8) {
				set_enable( (byte << 3) + bit , *data & 0x01);
				*data = *data>>1;
				bit++;
			}

			data++;
			byte++;
		}

		return GR_OK;
	}

	if (data_len < 3 || data_len > 252) {
		return GR_INVALID_DATA;
	}

	while (data_len) {
		set_position(*data, (data[1]<<8)|data[2]);
		data += 3;
		data_len -= 3;

		if (data_len < 3) {
			return GR_INVALID_DATA;
		}
	}

	return GR_OK;
}


GATE_RESULT init_servo_driver(void)
{
	for (uint8_t i=0; i < 16; i++) {
		set_enable(i, false);
		set_position(i, 1000);
    };

	// Prepare TIMER0 interrupt
	TCCR0 = 0x01;
	TCNT0 = 0;
	tmr = 0;
	TIMSK |= 0x01;

	return gate_driver_register(&driver);
}

