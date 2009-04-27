#ifndef MOTOR_H
#define MOTOR_H

#include <stdint.h>
#include <stdbool.h>

#include "debug.h"
#include "errors.h"
#include "i2c-registers.h"

enum {
    DIR_STOR = 0,
    DIR_FWD,
    DIR_BKWD,
};

void motor_init();
void motor_set(uint8_t pin, uint8_t val);
uint8_t motor_get(uint8_t pin);

#endif  // GPIO_H
