/*! I2C slave registers
 * Refrence: http://roboforum.ru/mediawiki/images/d/da/OR-AVR-M32-D-UG.pdf
 *
 * WARNING this is file genereted by cog.py
 * See: http://nedbatchelder.com/code/cog/
 * For ArchLinux users: install python-cog package from AUR
 *
 * for regenerate: cog.py -r i2c-registers.h
 */

#ifndef I2C_REGISTERS_H
#define I2C_REGISTERS_H

#include <stdint.h>
#include <stdbool.h>

/*! I2C registers
 *
 */
enum {
/*[[[cog
import cog

ident1 = " " * 4
ident2 = ident1 * 2
ports = {'A': {'name': range(0, 7+1), 'register': range(0x00, 0x07+1), },
    'C': {'name': range(7, 4-1, -1), 'register': range(0x08, 0x0b+1), },
    'B': {'name': range(2, 3+1), 'register': range(0x0c, 0x0d+1), },
    'D': {'name': range(5, 4-1, -1), 'register': range(0x0e, 0x0f+1), },
    }

for k, v in ports.items():
    vals = []
    for i in range(len(v['name'])):
        vals.append([k, v['name'][i], v['register'][i], ])

    cog.outl(ident1 + "// PORT/DDR/PIN %s" % k)
    for p, n, vl in vals:
        cog.outl(ident2 + "PORT_%s%s = 0x%02x," % (p, n, vl, ))
        cog.outl(ident2 + "DDR_%s%s  = 0x%02x," % (p, n, vl + 0x20, ))
        cog.outl(ident2 + "PIN_%s%s  = 0x%02x," % (p, n, vl + 0x80, ))
        cog.outl()
]]]*/
// PORT/DDR/PIN A
    PORT_A0 = 0x00,
    DDR_A0  = 0x20,
    PIN_A0  = 0x80,

    PORT_A1 = 0x01,
    DDR_A1  = 0x21,
    PIN_A1  = 0x81,

    PORT_A2 = 0x02,
    DDR_A2  = 0x22,
    PIN_A2  = 0x82,

    PORT_A3 = 0x03,
    DDR_A3  = 0x23,
    PIN_A3  = 0x83,

    PORT_A4 = 0x04,
    DDR_A4  = 0x24,
    PIN_A4  = 0x84,

    PORT_A5 = 0x05,
    DDR_A5  = 0x25,
    PIN_A5  = 0x85,

    PORT_A6 = 0x06,
    DDR_A6  = 0x26,
    PIN_A6  = 0x86,

    PORT_A7 = 0x07,
    DDR_A7  = 0x27,
    PIN_A7  = 0x87,

// PORT/DDR/PIN C
    PORT_C7 = 0x08,
    DDR_C7  = 0x28,
    PIN_C7  = 0x88,

    PORT_C6 = 0x09,
    DDR_C6  = 0x29,
    PIN_C6  = 0x89,

    PORT_C5 = 0x0a,
    DDR_C5  = 0x2a,
    PIN_C5  = 0x8a,

    PORT_C4 = 0x0b,
    DDR_C4  = 0x2b,
    PIN_C4  = 0x8b,

// PORT/DDR/PIN B
    PORT_B2 = 0x0c,
    DDR_B2  = 0x2c,
    PIN_B2  = 0x8c,

    PORT_B3 = 0x0d,
    DDR_B3  = 0x2d,
    PIN_B3  = 0x8d,

// PORT/DDR/PIN D
    PORT_D5 = 0x0e,
    DDR_D5  = 0x2e,
    PIN_D5  = 0x8e,

    PORT_D4 = 0x0f,
    DDR_D4  = 0x2f,
    PIN_D4  = 0x8f,

//[[[end]]]
};

#endif  // I2C_REGISTERS_H
