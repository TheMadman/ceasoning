/*
 * Ceasoning - Syntactic Sugar for Common C Tasks
 * Copyright (C) 2024   Marcus Harrison
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <csalt/resources.h>

#include "test_macros.h"

// pthread-specific, wat do?
// Maybe I need a platforms subdirectory for automated tests...
INIT_IMPL(
	int,
	pthread_mutex_init,
	ARGS(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr),
	ARGS(mutex, attr)
)

pthread_mutex_t *pthread_mutex_init_mutex_param = NULL;
const pthread_mutexattr_t *pthread_mutex_init_attr_param = NULL;
int pthread_mutex_init_return = 0;
int pthread_mutex_init_called = 0;

int pthread_mutex_init_stub(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr)
{
	pthread_mutex_init_mutex_param = mutex;
	pthread_mutex_init_attr_param = attr;
	pthread_mutex_init_called++;
	return pthread_mutex_init_return;
}

INIT_IMPL(int, pthread_mutex_destroy, pthread_mutex_t *mutex, mutex)

pthread_mutex_t *pthread_mutex_destroy_mutex_param = NULL;
int pthread_mutex_destroy_return = 0;
int pthread_mutex_destroy_called = 0;

int pthread_mutex_destroy_stub(pthread_mutex_t *mutex)
{
	pthread_mutex_destroy_mutex_param = mutex;
	pthread_mutex_destroy_called++;
	return pthread_mutex_destroy_return;
}

int main()
{
	SET_IMPL(pthread_mutex_init, pthread_mutex_init_stub);
	SET_IMPL(pthread_mutex_destroy, pthread_mutex_destroy_stub);

	struct csalt_resource_stub stub = csalt_resource_stub(0);
	struct csalt_resource_mutex mutex = csalt_resource_mutex(
		(csalt_resource *)&stub,
		NULL);

	csalt_resource_init((csalt_resource*)&mutex);
	if (pthread_mutex_init_called != 1)
		print_error_and_exit("init wasn't called the correct amount: %d", pthread_mutex_init_called);

	csalt_resource_deinit((csalt_resource*)&mutex);
	if (pthread_mutex_destroy_called != 1)
		print_error_and_exit("destroy wasn't called the correct amount: %d", pthread_mutex_destroy_called);
}
