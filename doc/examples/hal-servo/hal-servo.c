/* HAL Test
 * (test servo)
 */

#include <stdint.h>
#include <stdio.h>

#include <util/delay.h>

#include "hal/servo.h"

#define STEP 30
#define SMIN 650
#define SMAX 2200
#define SCH  12

/** Main
 */
int main(void)
{
	uint16_t p = 1000;
	bool op = true; 

	servo_init();
	asm volatile ("sei");
	for(;;) {
		p += (op) ? +STEP : -STEP;
		if (p >= SMAX) {
			op = false;
			_delay_ms(50);
		}
		if (p <= SMIN) {
			op = true;
			_delay_ms(50);
		}

		servo_set_position(SCH, p);
		_delay_ms(20);
	}

	return 0;
}
