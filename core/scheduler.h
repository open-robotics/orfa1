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
/** Sheduler core
 * @file scheduler.h
 *
 * @author Andrey Demenev
 * @author Vladimir Ermakov <vooon341@gmail.com>
 */

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>
#include <stdbool.h>
#include "common.h"

/**
 * @defgroup Sheduler Планировщик задач
 *
 * Простой кооперативный планировщик задач
 * 
 * Есть одна суперзадача, которая вызывается перед выполнением
 * любой другой задачи. Все остальные задачи выполняются по кольцу.
 */

/**@{*/

/**
 * Прототип функции процесса.
 * Задача должна возвращать управление, если ее событи еще не наступило.
 * При этом нужно учитывать, что в следующий раз функция получит управление
 * через недетерминированный промежуток времени.
 */
typedef void (*GATE_TASK_FUNC)(void);

/**
 * Конфигурация процесса.
 */
typedef struct GATE_TASK_ {
	GATE_TASK_FUNC task; /**< Функция процесса */
	struct GATE_TASK_* next; /**< Указатель на следующий процесс */
} GATE_TASK;

/**
 * Регистрирует процесс.
 * @param task указатель на структуру задачи.
 * @return GR_OK, если задача была добавлена.
 */
GATE_RESULT gate_task_register(GATE_TASK* task);

/**
 * Устанавливает суперзадачу.
 * @param task указатель на функцию суперзадачи.
 * @return GR_OK если задача была установлена.
 */
GATE_RESULT gate_supertask_register(GATE_TASK_FUNC task);

/**
 * Главный цикл задач.
 */
void gate_scheduler_loop(void);

#endif
