#include "ports.h"
#include "common.h"

static GATE_PORT* ports;

GATE_PORT* find_port(uint8_t number)
{
	GATE_PORT* cur = ports;
	while (cur) {
		if (cur->number == number) {
			return cur;
		}
		cur = cur->next;
	}
	return 0;
}

GATE_RESULT gate_port_register(uint8_t number, GATE_PORT* spec)
{
	if (!spec) {
		return GR_INVALID_ARG;
	}
	if (find_port(number)) {
		return GR_DUPLICATE_PORT;
	}
	spec->number = number;
	spec->busy_mask = spec->default_busy_mask;
	spec->next = ports;
	ports = spec;
	return GR_OK;
}

GATE_RESULT gate_port_config(uint8_t number, uint8_t mask, uint8_t value)
{
	GATE_PORT* port = find_port(number);
	if (!port) {
		return GR_INVALID_PORT;
	}
	uint8_t port_value = *(port->DDR);
	mask = mask & ~(port->default_busy_mask | port->busy_mask);
	port_value &= (value | mask);
	port_value |= (value & ~mask);
	*(port->DDR) = port_value;
	return GR_OK;
}

GATE_RESULT gate_port_write(uint8_t number, uint8_t mask, uint8_t value)
{
	GATE_PORT* port = find_port(number);
	if (!port) {
		return GR_INVALID_PORT;
	}
	uint8_t port_value = *(port->PORT);
	mask = mask & ~(port->default_busy_mask | port->busy_mask);
	port_value &= (value | mask);
	port_value |= (value & ~mask);
	*(port->PORT) = port_value;
	return GR_OK;
}

GATE_RESULT gate_port_read(uint8_t number, uint8_t* bits)
{
	GATE_PORT* port = find_port(number);
	if (!port) {
		return GR_INVALID_PORT;
	}
	*bits = *(port->PIN);
	return GR_OK;
}

GATE_RESULT gate_port_reserve(uint8_t number, uint8_t mask, uint8_t value)
{
	GATE_PORT* port = find_port(number);
	if (!port) {
		return GR_INVALID_PORT;
	}
	uint8_t port_value = port->busy_mask;
	mask = mask & ~port->default_busy_mask;
	port_value &= (value | mask);
	port_value |= (value & ~mask);
	port->busy_mask = port_value;
	return GR_OK;
}

