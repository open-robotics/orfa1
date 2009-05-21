/*
 *  ORFA -- Open Robotics Firmware Architecture
 *
 *  Copyright (c) 2009 Vladimir Ermakov, Andrey Demenev
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 *  THE SOFTWARE.
 *****************************************************************************/

#include <avr/io.h>
#include "driver.h"
#include "ports_driver.h"
#include "spi_driver.h"
#include <stdint.h>

uint8_t buf[10];

int main(void)
{
	init_ports_driver();
	init_spi_driver();

	// Примеры вызовов. В реальном шлюзе содержимое буфера
	// и номера регистров будет формировать парсер на основе
	// полученной от ПК информации.

	// Конфигурируем PORTC.5 как выход
	buf[0] = 0x20;
	gate_register_write(0x46, buf, 1);

	// Установка скорости SPI
	buf[0] = 0xFF;
	buf[1] = 0x04;
	gate_register_write(0x51, buf, 2);

	// Для канала 0 SPI, выбор устройства будет осуществляться выводом PORTB.4
	buf[0] = 0x00;
	buf[1] = 0x02;
	buf[2] = 0x04;
	gate_register_write(0x51, buf, 2);

	// Записываем в канал 0 SPI 4 байта, чередующиеся 0x55/0xAA
	buf[0] = 0x00;
	buf[1] = 0xAA;
	buf[2] = 0x55;
	buf[3] = 0xAA;
	buf[4] = 0x55;
	gate_register_write(0x50, buf, 5);

	// Читаем  данные, полученные в процессе последней операции записи SPI
	uint8_t bytes_read = 10;
	gate_register_read(0x50, buf, &bytes_read);
	// bytes_read будет содержать 5

	for(;;);
}
