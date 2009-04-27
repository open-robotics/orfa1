#include "gpio.h"

#ifdef AVR_IO

#include <avr/io.h>
#include <avr/interrupt.h>

const uint8_t *rports[PORT_D4 + 1] = {
    // PORT_A0 — PORT_A7
    &PORTA, &PORTA, &PORTA, &PORTA, &PORTA, &PORTA, &PORTA,
    // PORT_C7 — PORT_C4
    &PORTC, &PORTC, &PORTC, &PORTC,
    // PORT_B2 — PORT_B3
    &PORTB, &PORTB,
    // PORT_D5 — PORT_D4
    &PORTD, &PORTD,
};

const uint8_t *rpins[PORT_D4 + 1] = {
    // PORT_A0 — PORT_A7
    &PINA, &PINA, &PINA, &PINA, &PINA, &PINA, &PINA,
    // PORT_C7 — PORT_C4
    &PINC, &PINC, &PINC, &PINC,
    // PORT_B2 — PORT_B3
    &PINB, &PINB,
    // PORT_D5 — PORT_D4
    &PIND, &PIND,
};

const uint8_t *rddrs[PORT_D4 + 1] = {
    // DDR_A0 — DDR_A7
    &DDRA, &DDRA, &DDRA, &DDRA, &DDRA, &DDRA, &DDRA,
    // DDR_C7 — DDR_C4
    &DDRC, &DDRC, &DDRC, &DDRC,
    // DDR_B2 — DDR_B3
    &DDRB, &DDRB,
    // DDR_D5 — DDR_D4
    &DDRD, &DDRD,
};

const uint8_t pins[PORT_D4 + 1] = {
    // PORT_A0 — PORT_A7
    _BV(PA0), _BV(PA1), _BV(PA2), _BV(PA3), _BV(PA4), _BV(PA5), _BV(PA6), _BV(PA7),
    // PORT_C7 — PORT_C4
    _BV(PC7), _BV(PC6), _BV(PC5), _BV(PC4),
    // PORT_B2 — PORT_B3
    _BV(PB2), _BV(PB3),
    // PORT_D5 — PORT_D4
    _BV(PD5), _BV(PD4),
};

#endif // AVR_IO

uint8_t gpio_type[PORT_D4 + 1];
uint8_t gpio_state[PORT_D4 + 1];

uint8_t * reg;
uint8_t pinn;

void gpio_set(uint8_t pin, uint8_t val)
{
    debug("# >> gpio_set(%i, 0x%02x)\n", pin, val);

    gpio_state[pin] = val;

    #ifdef AVR_IO
    if(gpio_type[pin] == TYPE_IN || gpio_type[pin] == TYPE_OUT)
    {
        reg = rports[pin];
        pinn = pins[pin];
        if(val)
            *reg |= pinn;
        else
            *reg &= ~pinn;
    }
    #endif // AVR_IO
}

uint8_t gpio_get(uint8_t pin)
{
    uint8_t ret;

    #ifdef AVR_IO
    if(gpio_type[pin] == TYPE_IN || gpio_type[pin] == TYPE_OUT)
    {
        reg = rports[pin];
        pinn = pins[pin];
        ret = (*reg & pinn) && true;
    }
    else
    #endif // AVR_IO
    {
        ret = gpio_state[pin];
    }

    debug("# >> gpio_get(%i) -> %i\n", pin, ret);
    return ret;
}

void gpio_set_type(uint8_t pin, uint8_t val)
{
    bool ddr;

    debug("# >> gpio_set_type(%i, %i)\n", pin, val);

    pin -= DDR_A0;

    switch(val)
    {
        case TYPE_IN:
        case TYPE_OUT:
            ddr = val;
            break;

        case TYPE_SERVO:
            ddr = TYPE_OUT;
            break;

        case TYPE_ADC:
            ddr = TYPE_IN;
            if(PORT_A0 <= pin && pin <= PORT_A7)
                val = TYPE_IN;
            break;

        default:
            ddr = TYPE_IN;
            break;
    }

    gpio_type[pin] = val;

    #ifdef AVR_IO
    reg = rports[pin];
    pinn = pins[pin];
    if(ddr)
        *reg |= pinn;
    else
        *reg &= ~pinn;
    #endif // AVR_IO
}

uint8_t gpio_get_type(uint8_t pin)
{
    uint8_t ret = gpio_type[pin - DDR_A0];

    debug("# >> gpio_get_type(%i) -> %i\n", pin, ret);
    return ret;
}

