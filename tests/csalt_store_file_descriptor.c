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
	size_t result = csalt_store_size(fd);
	if (result != 0) {
		print_error("csalt_store_size() returned unexpected result for split pipe: %ld", result);
		exit(EXIT_FAILURE);
	}
	return EXIT_SUCCESS;
}

int split_file(csalt_store *fd, void *param)
{
	size_t result = csalt_store_size(fd);
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

	size_t read_result = read(pipe_fds[0], buffer, sizeof(buffer));

	if (strcmp(buffer, WRITE_BUFFER)) {
		print_error("read() doesn't match csalt_store_write(): \"%s\"", buffer);
		return EXIT_FAILURE;
	}

	memset(buffer, 0, sizeof(buffer));

	struct csalt_store_file_descriptor read_file = csalt_store_file_descriptor(pipe_fds[0]);
	csalt_store *read_store = (csalt_store *)&read_file;

	size_t write_result = write(pipe_fds[1], WRITE_BUFFER, sizeof(WRITE_BUFFER));
	csalt_store_read(read_store, buffer, sizeof(buffer));

	if (strcmp(buffer, WRITE_BUFFER)) {
		print_error("csalt_store_read() doesn't match write(): \"%s\"", buffer);
		return EXIT_FAILURE;
	}

	size_t pipe_size = csalt_store_size(read_store);
	if (pipe_size != 0) {
		print_error("Unexpected csalt_store_size() value for pipe: %ld", pipe_size);
		return EXIT_FAILURE;
	}

	csalt_store_split(read_store, 0, 10, split_pipe, 0);

	FILE *real_file = tmpfile();
	int real_file_fd = fileno(real_file);
	int trunc_result = ftruncate(real_file_fd, 10);

	struct csalt_store_file_descriptor csalt_tmpfile = csalt_store_file_descriptor(real_file_fd);
	csalt_store *tmpfile_store = (csalt_store *)&csalt_tmpfile;

	size_t real_file_size = csalt_store_size(tmpfile_store);
	if (real_file_size != 10) {
		print_error("Unexpected csalt_store_size() value for file: %ld", real_file_size);
		return EXIT_FAILURE;
	}

	csalt_store_split(tmpfile_store, 5, 10, split_file, 0);

	return EXIT_SUCCESS;
}
