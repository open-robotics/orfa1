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
/** Driver core
 * @file driver.h
 *
 * @author Andrey Demenev
 * @author Vladimir Ermakov <vooon341@gmail.com>
 */

#ifndef DRIVER_H
#define DRIVER_H

#include <stdint.h>
#include "common.h"

/**
 * @defgroup Drivers Интерфейс драйверов устройств
 *
 * Драйвер устройства состоит из структуры GATE_DRIVER, функций чтения и/или
 * записи в регистр, и (необязательно) функции инициализации драйвера.
 *
 * Регистры, используемые драйвером, должны содержаться в массиве, на который
 * указывает поле registers. Количество используемых регистров задается полем
 * num_registers. Драйвер не может использовать номера регистров произвольно -
 * регистры выделяются вызовом функции gate_allocate_register().
 *
 * Работа с устройством ввода/вывода заключается в записи массива данных в
 * регистры или чтения из регистра. То, как использовать массив данных,
 * переданных для записи, или как заполнять буфер данных при чтении, целиком
 * определяется драйвером устройства.
 *
 */

/**@{*/

/**
 * Прототип функции чтения данных из драйвера. Драйвер, реализующий чтение из
 * регистров, должен предоставлять функцию типа GATE_READ. Прочитанные данные
 * помещаются в буфер data. Количество байт для чтения задается переменной,
 * указатель на которую содержится в data_len. В эту же переменную помещается
 * количество фактически прочитанных байтов.
 *
 * @param reg Номер регистра.
 * @param data Указатель на буфер данных
 * @param data_len Указатель на переменную, значение которй задает количество
 *                 байт для чтения.
 *
 * @return GR_OK - если данные успешно прочитаны. Иначе - код ошибки. Функция
 * должна возвращать GR_NO_ACCESS, если чтение из запрошенного регистра не
 * поддерживается.
 */
typedef GATE_RESULT (*GATE_READ)(uint8_t reg, uint8_t* data, uint8_t* data_len);

/**
 * Прототип функции записи данных в драйвер.
 * Драйвер, реализующий запись в регистры, должен предоставлять функцию типа
 * GATE_READ.
 *
 * @param reg Номер регистра.
 * @param data Указатель на массив с данными.
 * @param data_len Количество байт для записи.
 *
 * @return GR_OK - если данные успешно записаны. Иначе - код ошибки. Функция
 *         должна возвращать GR_NO_ACCESS, если запись в запрошенный регистр
 *         не поддерживается.
 */
typedef GATE_RESULT (*GATE_WRITE)(uint8_t reg, uint8_t* data, uint8_t data_len);

/**
 * Прототип функции инициализации драйвера.
 * Функция инициализации драйвера вызывается при регистрации драйвера.
 *
 * @return Функция должна возвращать GR_OK при успешной инициализации, иначе -
 *         код ошибки
 */
typedef GATE_RESULT (*GATE_INIT)(void);

/**
 * Конфигурация драйвера устройств.
 */
typedef struct GATE_DRIVER_ {
	uint8_t* registers; /**< Массив с номерами регистров, обслуживаемых драйвером */
	uint8_t  num_registers; /**< Количество регистров */
	uint16_t uid; /**< Идентификатор драйвера (для интроспекции) */
	uint8_t major_version;
	uint8_t minor_version;
	struct GATE_DRIVER_* next; /*<< Указатель на следующий драйвер в списке */
	GATE_READ read; /**< Функция чтения */
	GATE_WRITE write; /**< Функция записи */
	GATE_INIT init; /**< Функция инициализации драйвера */
} GATE_DRIVER;

/**
 * Регистрирует драйвер устройства.
 * @param driver Указатель на структуру с описанием драйвера
 * @return GR_OK, если драйвер был успешно добавлен. GR_DUPLICATE_REGISTER,
 *         если один из регистров уже используется другим драйвером.Если 
 *         драйвер предоставляетт функцию инициализации, возможны и другие
 *         значения. Любое значение, отличное от GR_OK, означает, что драйвер
 *         не был добавлен, и использовать его регистры невозможно.
 */
GATE_RESULT gate_driver_register(GATE_DRIVER* driver);

/**
 * Чтение из регистра.
 * @param reg Номер регистра
 * @param data Указатель на буфер данных
 * @param data_len Указатель на переменную, значение которй задает количество байт для чтения.
 * @return GR_INVALID_REGISTER - если для указанного регистра нет
 *         зарегистрированного драйвера. GR_NO_ACCESS - если драйвер, связанный
 *         с регистром, не предоставляет функцию чтения или чтение из
 *         запрошенного регистра не поддерживается. Иначе - результат выполнения
 *         функции чтения драйвера.
 *
 * @see GATE_READ
 */
GATE_RESULT gate_register_read(uint8_t reg, uint8_t* data, uint8_t* data_len);

/**
 * Запись в регистр.
 * @param reg Номер регистра.
 * @param data Указатель на массив с данными.
 * @param data_len Количество байт для записи.
 *
 * @return GR_INVALID_REGISTER - если для указанного регистра нет
 *         зарегистрированного драйвера. GR_NO_ACCESS - если драйвер, связанный
 *         с регистром, не предоставляет функцию записи или запись в запрошенный
 *         регистр не поддерживается. Иначе - результат выполнения функции
 *         записи драйвера.
 *
 * @see GATE_WRITE
 */
GATE_RESULT gate_register_write(uint8_t reg, uint8_t* data, uint8_t data_len);

/**
 * Выделение регистра.
 * Выделяет регистр для использования драйвером.
 *
 * @return 0, если нет доступных регистров, иначе номер регистра.
 */
uint8_t gate_allocate_register(void);

/**@}*/

/**
 * Инициализация драйвера интроспекции
 */
void gate_init_introspection(void);

#endif
