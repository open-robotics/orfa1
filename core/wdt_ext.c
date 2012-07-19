/** Extended function of WatchDog Timer. Implementation file
 * @file wdt_ext.c
 *
 * @author Mad
 */

#include "wdt_ext.h"

/**
 *	WatchDog mode:
 *		0 - WDT not used yet;
 *		1 - WDT activated by main program, general mode
 *		2 - WDT activated by command W ("Connection WatchDog")
 */
static uint8_t	wd_mode = 0;

/** WatchDog turned on in main program
 */
static uint8_t	wd_common_on = 0;

/** WatchDog interval code for general mode (activated in main program)
 */
static uint8_t	wd_interval_code;

/** WDT Enable
 *
 * @param[in] wdt_code wdt interval (e.g. WSTO_1S)
 */
void wdt_enable_ext(uint8_t wdt_code)
{
	wd_interval_code = wdt_code;
	switch(wd_mode)
	{
		case 0: {
			wd_mode = wd_common_on = 1;
			wdt_enable(wdt_code);
		} break;
		case 1: {
			wd_common_on = 1;
			wdt_reset();
			wdt_disable();
			wdt_enable(wdt_code);
		} break;
		case 2: {
			wd_common_on = 1;
		}
	}
}

/** WDT Disable
 */
void wdt_disable_ext(void)
{
	switch(wd_mode)
	{
		case 1: {
			wdt_reset();
			wdt_disable();
			wd_mode = wd_common_on = 0;
		} break;
		case 2: {
			wd_common_on = 0;
		}
	}
}

/** WDT Reset
 */
void wdt_reset_ext(void)
{
	if(wd_mode == 1)
		wdt_reset();
}

/** Connection WDT enable
 *
 * @param[in] wdt_code wdt interval (WDTO_1S)
 */
void wdt_enable_extc(uint8_t wdt_code)
{
	if(wd_mode)
		wdt_reset();
	wdt_enable(wdt_code);
	wd_mode = 2;
}

/** Connection WDT disable
 */
void wdt_disable_extc(void)
{
	wdt_reset();
	if(wd_common_on)
	{
		wdt_enable(wd_interval_code);
		wd_mode = 1;
	}
	else
	{
		wdt_disable();
		wd_mode = 0;
	}
}

