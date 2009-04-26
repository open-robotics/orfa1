#ifdef AVR_IO

#include <avr/io.h>
#include <avr/interrupt.h>

#endif // AVR_IO

#include "gpio.h"

uint8_t gpio_type[PORT_D4 + 1];
uint8_t gpio_state[PORT_D4 + 1];


void gpio_set(uint8_t pin, uint8_t val)
{
    debug("# >> gpio_set(%i, 0x%02x)\n", pin, val);

    gpio_state[pin] = val;

    #ifdef AVR_IO
    if(gpio_type[pin] == 0 || gpio_type[pin] == 1)
    {
        uint8_t *port;

        switch(pin)
        {
            case PORT_A0:
            case PORT_A1:
            case PORT_A2:
            case PORT_A3:
            case PORT_A4:
            case PORT_A5:
            case PORT_A6:
            case PORT_A7:
                port = &PORTA;
                break;

            case PORT_C7:
                port = &PORTC;
                pin = 7;
                break;
            case PORT_C6:
                port = &PORTC;
                pin = 6;
                break;
            case PORT_C5:
                port = &PORTC;
                pin = 5;
                break;
            case PORT_C4:
                port = &PORTC;
                pin = 4;
                break;

            case PORT_B2:
            case PORT_B3:
                port = &PORTB;
                pin = pin - PORT_B2;
                break;

            case PORT_D5:
                port = &PORTD;
                pin = 5;
                break;
            case PORT_D4:
                port = &PORTD;
                pin = 4;
                break;
        }

        if(val)
            *port |= (1<<pin);
        else
            *port &= ~(1<<pin);
    }
    #endif // AVR_IO
}

uint8_t gpio_get(uint8_t pin)
{
    uint8_t ret = gpio_state[pin];

    #ifdef AVR_IO
    if(gpio_type[pin] == 0 || gpio_type[pin] == 1)
    {
        uint8_t *port;

        switch(pin)
        {
            case PORT_A0:
            case PORT_A1:
            case PORT_A2:
            case PORT_A3:
            case PORT_A4:
            case PORT_A5:
            case PORT_A6:
            case PORT_A7:
                port = &PORTA;
                break;

            case PORT_C7:
                port = &PORTC;
                pin = 7;
                break;
            case PORT_C6:
                port = &PORTC;
                pin = 6;
                break;
            case PORT_C5:
                port = &PORTC;
                pin = 5;
                break;
            case PORT_C4:
                port = &PORTC;
                pin = 4;
                break;

            case PORT_B2:
            case PORT_B3:
                port = &PORTB;
                pin = pin - PORT_B2;
                break;

            case PORT_D5:
                port = &PORTD;
                pin = 5;
                break;
            case PORT_D4:
                port = &PORTD;
                pin = 4;
                break;
        }

        if(*port & (1<<pin))
            ret = true;
        else
            ret = false;
    }
    #endif // AVR_IO

    debug("# >> gpio_get(%i) -> %i\n", pin, ret);
    return ret;
}

void gpio_set_type(uint8_t pin, uint8_t val)
{
    debug("# >> gpio_set_type(%i, %i)\n", pin, val);

    pin -= DDR_A0;
    gpio_type[pin] = val;

    #ifdef AVR_IO
    uint8_t *port;

    switch(pin)
    {
        case PORT_A0:
        case PORT_A1:
        case PORT_A2:
        case PORT_A3:
        case PORT_A4:
        case PORT_A5:
        case PORT_A6:
        case PORT_A7:
            port = &DDRA;
            break;

        case PORT_C7:
            port = &DDRC;
            pin = 7;
            break;
        case PORT_C6:
            port = &DDRC;
            pin = 6;
            break;
        case PORT_C5:
            port = &DDRC;
            pin = 5;
            break;
        case PORT_C4:
            port = &DDRC;
            pin = 4;
            break;

        case PORT_B2:
        case PORT_B3:
            port = &DDRB;
            pin = pin - PORT_B2;
            break;

        case PORT_D5:
            port = &DDRD;
            pin = 5;
            break;
        case PORT_D4:
            port = &DDRD;
            pin = 4;
            break;
    }

    if(val)
        *port |= (1<<pin);
    else
        *port &= ~(1<<pin);
    #endif // AVR_IO
}

uint8_t gpio_get_type(uint8_t pin)
{
    uint8_t ret = gpio_type[pin - DDR_A0];

    debug("# >> gpio_get_type(%i) -> %i\n", pin, ret);
    return ret;
}

