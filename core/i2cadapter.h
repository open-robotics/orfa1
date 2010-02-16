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
/** I2C adapter core
 * @file i2cadapter.h
 *
 * @author Andrey Demenev
 * @author Vladimir Ermakov <vooon341@gmail.com>
 */

#ifndef I2CADAPTER_H
#define I2CADAPTER_H

#include "common.h"

/**
 * @defgroup I2CAdapters Интерфейс I2C адаптеров устройств
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

/** Прототип функции чтения данных из драйвера.
 * Драйвер, реализующий чтение из
 * регистров, должен предоставлять функцию типа GATE_READ. Прочитанные данные
 * помещаются в буфер data. Количество байт для чтения задается переменной,
 * указатель на которую содержится в data_len. В эту же переменную помещается
 * количество фактически прочитанных байтов.
 *
 * @param[in]  reg Номер регистра.
 * @param[out] data Указатель на буфер данных
 * @param[out] data_len Указатель на переменную, значение которй
 *                      задает количество байт для чтения.
 *
 * @return GR_OK - если данные успешно прочитаны. Иначе - код ошибки. Функция
 * должна возвращать GR_NO_ACCESS, если чтение из запрошенного регистра не
 * поддерживается.
 */
typedef GATE_RESULT (*GATE_READ)(uint8_t reg, uint8_t* data, uint8_t* data_len);

/** Прототип функции записи данных в драйвер.
 * Драйвер, реализующий запись в регистры, должен предоставлять функцию типа
 * GATE_READ.
 *
 * @param[in] reg Номер регистра.
 * @param[in] data Указатель на массив с данными.
 * @param[in] data_len Количество байт для записи.
 *
 * @return GR_OK - если данные успешно записаны. Иначе - код ошибки. Функция
 *         должна возвращать GR_NO_ACCESS, если запись в запрошенный регистр
 *         не поддерживается.
 */
typedef GATE_RESULT (*GATE_WRITE)(uint8_t reg, uint8_t* data, uint8_t data_len);

/** Конфигурация драйвера устройств.
 */
typedef struct GATE_I2CADAPTER_ GATE_I2CADAPTER;
struct GATE_I2CADAPTER_ {
	uint16_t uid;            /**< Идентификатор драйвера (для интроспекции) */
	uint8_t major_version;   /**< Major version */
	uint8_t minor_version;   /**< Minor version */
	GATE_READ read;          /**< Функция чтения */
	GATE_WRITE write;        /**< Функция записи */
	uint8_t start_register;  /**< Начальный регистр, из диапазона регистров обслуживаемых драйвером */
	uint8_t  num_registers;  /**< Количество регистров */

	struct GATE_I2CADAPTER_* next; /*<< Указатель на следующий драйвер в списке */
};

/** Регистрирует драйвер устройства.
 * @param[in] driver Указатель на структуру с описанием драйвера
 * @return GR_OK, если драйвер был успешно добавлен. GR_DUPLICATE_REGISTER,
 *         если один из регистров уже используется другим драйвером.Если 
 *         драйвер предоставляетт функцию инициализации, возможны и другие
 *         значения. Любое значение, отличное от GR_OK, означает, что драйвер
 *         не был добавлен, и использовать его регистры невозможно.
 */
GATE_RESULT gate_i2cadapter_register(GATE_I2CADAPTER* adapter);

/** Чтение из регистра.
 * @param[in]  reg Номер регистра
 * @param[out] data Указатель на буфер данных
 * @param[out] data_len Указатель на переменную, значение которй задает количество байт для чтения.
 * @return GR_INVALID_REGISTER - если для указанного регистра нет
 *         зарегистрированного драйвера. GR_NO_ACCESS - если драйвер, связанный
 *         с регистром, не предоставляет функцию чтения или чтение из
 *         запрошенного регистра не поддерживается. Иначе - результат выполнения
 *         функции чтения драйвера.
 *
 * @see GATE_READ
 */
GATE_RESULT gate_register_read(uint8_t reg, uint8_t* data, uint8_t* data_len);

/** Запись в регистр.
 * @param[in] reg Номер регистра.
 * @param[in] data Указатель на массив с данными.
 * @param[in] data_len Количество байт для записи.
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

/**@}*/

/** Инициализация драйвера интроспекции
 */
void gate_init_introspection(void);

#endif
