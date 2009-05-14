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

#ifndef PORTS_H
#define PORTS_H

#include <stdint.h>
#include "common.h"


/**
 * @defgroup Ports Интерфейс портов ввода/вывода
 *
 * Конфигурация отдельного порта описывается в структуре GATE_PORT. Доступ к
 * портам МК осуществляется через адресное пространство ОЗУ, что позволяет
 * изменять конфигурацию портов без изменения функций, осуществляющих собственно
 * доступ к портам.
 *
 * Каждый используемый порт должен быть зарегистрирован вызовом функции
 * gate_port_register().
 *
 */

/**@{*/

/**
 * Конфигурация порта ввода/вывода
 */
typedef struct GATE_PORT_ {
	uint8_t  number;/**< Уникальный номер, однозначно идентифицирующий порт */
	uint8_t* PORT;	/**< Адрес регистра PORTx в адресном пространстве ОЗУ */
	uint8_t* PIN;	/**< Адрес регистра PINx в адресном пространстве ОЗУ */
	uint8_t* DDR;	/**< Адрес регистра DDRx в адресном пространстве ОЗУ */
	uint8_t  default_busy_mask; /**< Маска, определяющая биты порта, которые не могут использоваться (например используются для внутренних целей контроллера) */
	uint8_t  busy_mask; /**< Маска, определяющая биты порта, которые используются драйверами устройств */
	struct GATE_PORT_* next;
} GATE_PORT;

/**
 * Регистрация порта.
 * Связывает номер порта с конкретными регистрами ввода/вывода.
 * @param number Идентификатор порта
 * @param spec Указатель на конфигурацию порта
 */
GATE_RESULT gate_port_register(uint8_t number, GATE_PORT* spec);

/**
 * Настройка режима ввода/вывода порта. 
 * @note Режим разрядов порта, зарезервированных для использования драйвером
 * устройства, не меняется.
 *
 * @param number Идентификатор порта
 * @param mask Маска, определяющая, режим каких выводов порта изменить.
 * @param value Биты режима. Единица - вывод работает как выход, 0 - как вход.
 *
 */
GATE_RESULT gate_port_config(uint8_t number, uint8_t mask, uint8_t value);

/** Запись в порт.
 * @note Состояние разрядов порта, зарезервированных для использования драйвером
 * устройства, не меняется.
 *
 * @param number Идентификатор порта
 * @param mask Маска, определяющая, состояние каких выводов порта изменить.
 * @param value Значение выводимое в порт.
 */
GATE_RESULT gate_port_write(uint8_t number, uint8_t mask, uint8_t value);

/** Чтение из порта
 * @param number Идентификатор порта
 * @param bits Указатель на переменную, в которую помещается считанное значение.
 */
GATE_RESULT gate_port_read(uint8_t number, uint8_t* bits);

/** Переключение отдельных битов порта на выполнение специальных функций.
 * Эта функция вызывается драйверами устройств для резервирования определенных
 * разрядов порта под собственные нужды. На зарезервированные разряды порта не
 * влияют вызовы gate_port_config() и gate_port_write().
 *
 * @param number Идентификатор порта
 * @param mask Маска, определяющая, режим каких выводов порта изменить.
 * @param value Биты режима. Единица - вывод используется драйвером устройства, 0 - вывод работает как цифровая линия ввода/вывода.
 */
GATE_RESULT gate_port_reserve(uint8_t number, uint8_t mask, uint8_t value);

GATE_PORT* find_port(uint8_t number);

/**@}*/

#endif
