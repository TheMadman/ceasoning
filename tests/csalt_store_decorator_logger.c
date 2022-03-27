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

#include "csalt/stores.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

#include "test_macros.h"


#define LOG_LABEL_READ_ERROR "stub read failed"
#define LOG_LABEL_WRITE_ERROR "stub write failed"

#define LOG_LABEL_READ_SUCCESS "stub read success"
#define LOG_LABEL_WRITE_SUCCESS "stub write success"

#define LOG_LABEL_READ_ZERO "stub read zero bytes"
#define LOG_LABEL_WRITE_ZERO "stub write zero bytes"

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

	struct csalt_store_stub stub = csalt_store_stub(20);
	struct csalt_store_stub error = csalt_store_stub_error();

	struct csalt_store_log_message errors[] = {
		csalt_store_log_message(csalt_store_read, LOG_LABEL_READ_ERROR),
		csalt_store_log_message(csalt_store_write, LOG_LABEL_WRITE_ERROR),
	};

	struct csalt_store_log_message successes[] = {
		csalt_store_log_message(csalt_store_read, LOG_LABEL_READ_SUCCESS),
		csalt_store_log_message(csalt_store_write, LOG_LABEL_WRITE_SUCCESS),
	};

	struct csalt_store_log_message zero_bytes[] = {
		csalt_store_log_message(csalt_store_read, LOG_LABEL_READ_ZERO),
		csalt_store_log_message(csalt_store_write, LOG_LABEL_WRITE_ZERO),
	};

	struct csalt_store_decorator_logger
		logger = csalt_store_decorator_logger(
			(csalt_store *)&stub,
			write_fd,
			errors,
			successes,
			zero_bytes
		);

	struct csalt_store_decorator_logger
		error_logger = csalt_store_decorator_logger(
			csalt_store(&error),
			write_fd,
			errors,
			successes,
			zero_bytes
		);

	csalt_store *store = (csalt_store *)&logger;
	csalt_store *error_store = csalt_store(&error_logger);

	char buffer[1024] = { 0 };

	csalt_store_read(error_store, 0, 1);

	ssize_t read_result = read(read_fd, buffer, sizeof(buffer) - 1);

	if (read_result < 0) {
		perror("read()");
		return EXIT_TEST_ERROR;
	}

	char *result = strstr(buffer, LOG_LABEL_READ_ERROR);
	if (!result) {
		print_error("\"%s\" was not found in log output: %s", LOG_LABEL_READ_ERROR, buffer);
		return EXIT_FAILURE;
	}

	csalt_store_write(error_store, 0, 1);

	memset(buffer, 0, sizeof(buffer));
	read_result = read(read_fd, buffer, sizeof(buffer) - 1);

	if (read_result < 0) {
		perror("read()");
		return EXIT_TEST_ERROR;
	}

	result = strstr(buffer, LOG_LABEL_WRITE_ERROR);
	if (!result) {
		print_error("\"%s\" was not found in log output: %s", LOG_LABEL_WRITE_ERROR, buffer);
		return EXIT_FAILURE;
	}

	csalt_store_read(store, 0, 10);

	memset(buffer, 0, sizeof(buffer));
	read_result = read(read_fd, buffer, sizeof(buffer) - 1);

	if (read_result < 0) {
		perror("read()");
		return EXIT_TEST_ERROR;
	}

	result = strstr(buffer, LOG_LABEL_READ_SUCCESS);

	if (!result) {
		print_error("\"%s\" was not found in log output: %s", LOG_LABEL_READ_SUCCESS, buffer);
		return EXIT_FAILURE;
	}

	csalt_store_write(store, 0, 10);

	memset(buffer, 0, sizeof(buffer));
	read_result = read(read_fd, buffer, sizeof(buffer) - 1);

	if (read_result < 0) {
		perror("read()");
		return EXIT_TEST_ERROR;
	}

	result = strstr(buffer, LOG_LABEL_WRITE_SUCCESS);

	if (!result) {
		print_error("\"%s\" was not found in log output: %s", LOG_LABEL_WRITE_SUCCESS, buffer);
		return EXIT_FAILURE;
	}


	csalt_store_read(store, 0, 0);

	memset(buffer, 0, sizeof(buffer));
	read_result = read(read_fd, buffer, sizeof(buffer) - 1);

	if (read_result < 0) {
		perror("read()");
		return EXIT_TEST_ERROR;
	}

	result = strstr(buffer, LOG_LABEL_READ_ZERO);
	if (!result) {
		print_error("\"%s\" was not found in log output: %s", LOG_LABEL_READ_ZERO, buffer);
		return EXIT_FAILURE;
	}

	csalt_store_write(store, 0, 0);

	memset(buffer, 0, sizeof(buffer));
	read_result = read(read_fd, buffer, sizeof(buffer) - 1);

	if (read_result < 0) {
		perror("read()");
		return EXIT_TEST_ERROR;
	}

	result = strstr(buffer, LOG_LABEL_WRITE_ZERO);

	if (!result) {
		print_error("\"%s\" was not found in log output: %s", LOG_LABEL_WRITE_ZERO, buffer);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
