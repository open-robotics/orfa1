/*! Motor control functions
 */
// vim:set ts=4 sw=4 et:

#include "motor.h"

#ifdef AVR_IO

#include <avr/io.h>
#include <avr/interrupt.h>

const uint8_t *regs[4] = {
    // MOROR 1
    &PORTB, &OCR1A,
    // MOTOR 2
    &PORTB, &OCR2B,
};

static const uint8_t pins[PORT_D4 + 1] = {
    // MOTOR 1
    _BV(PB0), 0,
    // MOTOR 2
    _BV(PB1), 0,
};

#define DDRB_MASK _BV(PB0)|_BV(PB1)
#define DDRD_MASK _BV(PD4)|_BV(PD5)

#endif // AVR_IO

static uint8_t * reg;
static uint8_t pinn;

void motor_init()
{
    debug("# motor_init()\n");
#ifdef AVR_IO
    DDRB |= DDRB_MASK;
    DDRD |= DDRD_MASK;
#endif // AVR_IO
}

void motor_set(uint8_t pin, uint8_t val)
{
    debug("# motor_set(0x%02x, 0x%02x)\n", pin, val);
}

uint8_t motor_get(uint8_t pin)
{
    debug("# motor_get(0x%02x)\n", pin);
    return 0;
}
