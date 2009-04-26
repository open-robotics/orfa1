#ifndef GPIO_H
#define GPIO_H

#include <stdint.h>
#include <stdbool.h>

#include "debug.h"
#include "errors.h"
#include "i2c-registers.h"

extern uint8_t gpio_type[PORT_D4 + 1];
extern uint8_t gpio_state[PORT_D4 + 1];

void gpio_set(uint8_t pin, uint8_t val);
uint8_t gpio_get(uint8_t pin);
void gpio_set_type(uint8_t pin, uint8_t val);
uint8_t gpio_get_type(uint8_t pin);

#endif  // GPIO_H
