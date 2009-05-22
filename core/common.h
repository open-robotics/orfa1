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
/** Core common defines
 * @file common.h
 *
 * @author Andrey Demenev
 * @author Vladimir Ermakov <vooon341@gmail.com>
 */

#ifndef GR_COMMON_H
#define GR_COMMON_H

#define NUM_ELEMENTS(array) (sizeof(array) / sizeof(*(array)))


/**
 * @defgroup Errors Коды ошибок
 */

/**@{*/

/**
 * Код результата выполнения функций
 **/

typedef enum {
	GR_OK = 0,				/**< Успешно */
	GR_INVALID_ARG,			/**< Неверный аргумент */
	GR_DUPLICATE_PORT,		/**< Порт переопределен */
	GR_INVALID_PORT,		/**< Неизвестный порт */
	GR_INVALID_REGISTER,	/**< Неопределенный регистр */
	GR_ALLOCATE_REGISTER,	/**< Невозможно выделить область регистров */
	GR_NO_ACCESS,			/**< Нет доступа к регистру */
	GR_INVALID_DATA,		/**< Драйвер устройства не смог интерпретировать переданные данные */
} GATE_RESULT;

/**@}*/


#endif

