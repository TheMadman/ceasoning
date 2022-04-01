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
#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>

#include "test_macros.h"

#define WRITE_BUFFER "Hello World!"
#define FILE_SIZE 32

int split_pipe(csalt_store *fd, void *param)
{
	(void)param;
	ssize_t result = csalt_store_size(fd);
	if (result != 0) {
		print_error("csalt_store_size() returned unexpected result for split pipe: %ld", result);
		exit(EXIT_FAILURE);
	}
	return EXIT_SUCCESS;
}

int split_file(csalt_store *fd, void *param)
{
	(void)param;
	ssize_t result = csalt_store_size(fd);
	if (result != 5) {
		print_error("csalt_store_size() returned unexpected result for split pipe: %ld", result);
		exit(EXIT_FAILURE);
	}
	return EXIT_SUCCESS;
}

int main()
{
	int pipe_fds[2];
	int pipe_result = pipe(pipe_fds);
	if (pipe_result < 0) {
		perror("pipe");
		return EXIT_TEST_ERROR;
	}

	struct csalt_store_file_descriptor write_file = csalt_store_file_descriptor(pipe_fds[1]);
	csalt_store *write_store = (csalt_store *)&write_file;

	char buffer[sizeof(WRITE_BUFFER)] = { 0 };

	csalt_store_write(write_store, WRITE_BUFFER, sizeof(WRITE_BUFFER));

	ssize_t read_result = read(pipe_fds[0], buffer, sizeof(buffer));

	if (read_result < 0) {
		perror("read()");
		return EXIT_TEST_ERROR;
	}

	if (strcmp(buffer, WRITE_BUFFER)) {
		print_error("read() doesn't match csalt_store_write(): \"%s\"", buffer);
		return EXIT_FAILURE;
	}

	memset(buffer, 0, sizeof(buffer));

	struct csalt_store_file_descriptor read_file = csalt_store_file_descriptor(pipe_fds[0]);
	csalt_store *read_store = (csalt_store *)&read_file;

	ssize_t write_result = write(pipe_fds[1], WRITE_BUFFER, sizeof(WRITE_BUFFER));

	if (write_result != sizeof(WRITE_BUFFER)) {
		print_error(
			"write() returned unexpected value, expected: %ld actual: %ld",
			sizeof(WRITE_BUFFER),
			write_result
		);
		return EXIT_TEST_ERROR;
	}

	csalt_store_read(read_store, buffer, sizeof(buffer));

	if (strcmp(buffer, WRITE_BUFFER)) {
		print_error("csalt_store_read() doesn't match write(): \"%s\"", buffer);
		return EXIT_FAILURE;
	}

	ssize_t pipe_size = csalt_store_size(read_store);
	if (pipe_size != 0) {
		print_error("Unexpected csalt_store_size() value for pipe: %ld", pipe_size);
		return EXIT_FAILURE;
	}

	csalt_store_split(read_store, 0, 10, split_pipe, 0);

	FILE *real_file = tmpfile();
	int real_file_fd = fileno(real_file);

	struct csalt_store_file_descriptor csalt_tmpfile = csalt_store_file_descriptor(real_file_fd);
	csalt_store *tmpfile_store = (csalt_store *)&csalt_tmpfile;

	ssize_t real_file_size = csalt_store_size(tmpfile_store);
	if (real_file_size != 0) {
		print_error("Unexpected csalt_store_size() value for file: %ld", real_file_size);
		return EXIT_FAILURE;
	}

	(void)csalt_store_write(
		tmpfile_store,
		WRITE_BUFFER,
		sizeof(WRITE_BUFFER)
	);

	real_file_size = csalt_store_size(tmpfile_store);
	if (real_file_size != sizeof(WRITE_BUFFER)) {
		print_error("Unexpected csalt_store_size() value for file: %ld", real_file_size);
		return EXIT_FAILURE;
	}

	// Test if the file is being correctly overwritten
	// or if it's being appended to because of internal
	// cursor shenanigans
	(void)csalt_store_write(
		tmpfile_store,
		WRITE_BUFFER,
		sizeof(WRITE_BUFFER)
	);

	real_file_size = csalt_store_size(tmpfile_store);
	if (real_file_size != sizeof(WRITE_BUFFER)) {
		print_error("Unexpected csalt_store_size() value for file: %ld", real_file_size);
		return EXIT_FAILURE;
	}

	memset(buffer, 0, sizeof(buffer));
	(void)csalt_store_read(
		tmpfile_store,
		buffer,
		sizeof(buffer)
	);

	if (strcmp(buffer, WRITE_BUFFER)) {
		print_error("the thing is broke");
		return EXIT_FAILURE;
	}

	csalt_store_split(tmpfile_store, 5, 10, split_file, 0);

	return EXIT_SUCCESS;
}
