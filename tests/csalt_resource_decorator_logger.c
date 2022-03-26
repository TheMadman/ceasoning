/*
 * Ceasoning - Syntactic Sugar for Common C Tasks
 * Copyright (C) 2022   Marcus Harrison
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

#include "csalt/decoratorresources.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

#include "test_macros.h"

#define INIT_FAIL_MESSAGE "Stub init failed"

int main()
{
	int pipe_fds[2];
	int pipe_status = pipe(pipe_fds);
	if (pipe_status) {
		perror("pipe");
		return EXIT_TEST_ERROR;
	}

	int read_fd = pipe_fds[0];
	int write_fd = pipe_fds[1];
	char buffer[1024] = { 0 };

	struct csalt_resource_stub stub_fail = csalt_resource_stub_fail();

	struct csalt_store_log_message messages[] = {
		csalt_store_log_message(csalt_resource_init, INIT_FAIL_MESSAGE),
	};

	struct csalt_resource_decorator_logger logger = csalt_resource_decorator_logger_errors(
		(csalt_resource *)&stub_fail,
		write_fd,
		messages
	);

	(void)csalt_resource_init((csalt_resource *)&logger);
	read(read_fd, buffer, sizeof(buffer));

	if (!strstr(buffer, INIT_FAIL_MESSAGE)) {
		print_error(
			"Message read from log output didn't contain \"%s\", contents was: %s",
			INIT_FAIL_MESSAGE,
			buffer
		);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
