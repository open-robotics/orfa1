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

#include "common.h"
#include "scheduler.h"
#include <stdint.h>

static GATE_TASK* tasks;
static GATE_TASK_FUNC supertask;

GATE_RESULT gate_task_register(GATE_TASK* task)
{
	if (!tasks) {
		tasks = task;
		tasks->next = task;
	}

	task->next = tasks->next;
	tasks->next = task;

	return GR_OK;
}

GATE_RESULT gate_supertask_register(GATE_TASK_FUNC task)
{
	supertask = task;

	return GR_OK;
}

void gate_scheduler_loop(void)
{
	for (;;) {
		if (supertask) {
			supertask();
		}

		if (tasks) {
			if (tasks->task) {
				tasks->task();
			}

			if (!tasks->next) {
				// error!
				break;
			}
		
			tasks = tasks->next;
		}
	}

	for (;;) {
		// catch errors
		// for watchdog timer
		// puts("Error");
	}
}

