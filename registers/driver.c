#include "driver.h"
#include <stdint.h>

static GATE_DRIVER* drivers;

static GATE_DRIVER* find_driver(uint8_t reg)
{
	GATE_DRIVER* driver = drivers;
	while (driver) {
		uint8_t i = driver->num_registers;
		uint8_t* cur = driver->registers;
		while (i) {
			if (*cur == reg) {
				return driver;
			}
			i--;
			cur++;
		}
		driver = driver->next;
	}
	return 0;
}

GATE_RESULT gate_register_read(uint8_t reg, uint8_t* data, uint8_t* data_len)
{
	GATE_DRIVER* driver = find_driver(reg);
	if (driver) {
		if (!driver->read) {
			return GR_NO_ACCESS;
		}
		return driver->read(reg, data, data_len);
	}
	return GR_INVALID_REGISTER;
}

GATE_RESULT gate_register_write(uint8_t reg, uint8_t* data, uint8_t data_len)
{
	GATE_DRIVER* driver = find_driver(reg);
	if (driver) {
		if (!driver->write) {
			return GR_NO_ACCESS;
		}
		return driver->write(reg, data, data_len);
	}
	return GR_INVALID_REGISTER;
}

GATE_RESULT gate_driver_register(GATE_DRIVER* driver)
{
	uint8_t i = driver->num_registers;
	while (i) {
		if (find_driver(driver->registers[i])) {
			return GR_DUPLICATE_REGISTER;
		}
		i--;
	}
	GATE_RESULT res = GR_OK;
	if (driver->init) {
		res = driver->init();
	}
	if (res == GR_OK) {
		driver->next = drivers;
		drivers = driver;
	}
	return res;
}

