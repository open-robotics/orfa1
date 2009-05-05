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
	GR_DUPLICATE_REGISTER,	/**< Попытка использования драйвером регистра, занятого другим драйвером */
	GR_NO_ACCESS,			/**< Нет доступа к регистру */
	GR_INVALID_DATA,		/**< Драйвер устройства не смог интерпретировать переданные данные */
} GATE_RESULT;

/**@}*/


#endif

