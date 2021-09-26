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

	struct csalt_store_log_message errors[] = {
		{ csalt_store_read, LOG_LABEL_READ_ERROR, },
		{ csalt_store_write, LOG_LABEL_WRITE_ERROR, },
	};

	struct csalt_store_log_message successes[] = {
		{ csalt_store_read, LOG_LABEL_READ_SUCCESS, },
		{ csalt_store_write, LOG_LABEL_WRITE_SUCCESS, },
	};

	struct csalt_store_log_message zero_bytes[] = {
		{ csalt_store_read, LOG_LABEL_READ_ZERO, },
		{ csalt_store_write, LOG_LABEL_WRITE_ZERO, },
	};

	struct csalt_store_decorator_logger logger =
		csalt_store_decorator_logger(
			(csalt_store *)&stub,
			write_fd,
			errors,
			successes,
			zero_bytes
		);

	csalt_store *store = (csalt_store *)&logger;

	char buffer[1024] = { 0 };

	csalt_store_read(store, 0, -1);

	size_t read_len = read(read_fd, buffer, sizeof(buffer) - 1);
	char *result = strstr(buffer, LOG_LABEL_READ_ERROR);
	if (!result) {
		print_error("\"%s\" was not found in log output: %s", LOG_LABEL_READ_ERROR, buffer);
		return EXIT_FAILURE;
	}

	csalt_store_write(store, 0, -1);

	memset(buffer, 0, sizeof(buffer));
	read_len = read(read_fd, buffer, sizeof(buffer) - 1);
	result = strstr(buffer, LOG_LABEL_WRITE_ERROR);
	if (!result) {
		print_error("\"%s\" was not found in log output: %s", LOG_LABEL_WRITE_ERROR, buffer);
		return EXIT_FAILURE;
	}

	csalt_store_read(store, 0, 10);

	memset(buffer, 0, sizeof(buffer));
	read_len = read(read_fd, buffer, sizeof(buffer) - 1);
	result = strstr(buffer, LOG_LABEL_READ_SUCCESS);

	if (!result) {
		print_error("\"%s\" was not found in log output: %s", LOG_LABEL_READ_SUCCESS, buffer);
		return EXIT_FAILURE;
	}

	csalt_store_write(store, 0, 10);

	memset(buffer, 0, sizeof(buffer));
	read_len = read(read_fd, buffer, sizeof(buffer) - 1);
	result = strstr(buffer, LOG_LABEL_WRITE_SUCCESS);

	if (!result) {
		print_error("\"%s\" was not found in log output: %s", LOG_LABEL_WRITE_SUCCESS, buffer);
		return EXIT_FAILURE;
	}


	csalt_store_read(store, 0, 0);

	memset(buffer, 0, sizeof(buffer));
	read_len = read(read_fd, buffer, sizeof(buffer) - 1);
	result = strstr(buffer, LOG_LABEL_READ_ZERO);
	if (!result) {
		print_error("\"%s\" was not found in log output: %s", LOG_LABEL_READ_ZERO, buffer);
		return EXIT_FAILURE;
	}

	csalt_store_write(store, 0, 0);

	memset(buffer, 0, sizeof(buffer));
	read_len = read(read_fd, buffer, sizeof(buffer) - 1);
	result = strstr(buffer, LOG_LABEL_WRITE_ZERO);

	if (!result) {
		print_error("\"%s\" was not found in log output: %s", LOG_LABEL_WRITE_ZERO, buffer);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
