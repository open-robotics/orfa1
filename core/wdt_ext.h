/** Extended function of WatchDog Timer. Header file
 * @file wdt_ext.h
 *
 * @author Mad
 */

#ifndef WDT_EXT_H
#define WDT_EXT_H

#include <stdint.h>
#include <avr/wdt.h>

// ----- For main program ------------------------------------------
void wdt_enable_ext(uint8_t);
void wdt_disable_ext(void);
void wdt_reset_ext(void);

// ----- For connection watchdog -----------------------------------
void wdt_enable_extc(uint8_t);
void wdt_disable_extc(void);
#define wdt_reset_extc() __asm__ __volatile__ ("wdr")

#endif
