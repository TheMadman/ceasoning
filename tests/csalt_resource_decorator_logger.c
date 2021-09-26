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
		{ csalt_resource_init, INIT_FAIL_MESSAGE },
	};

	struct csalt_resource_decorator_logger logger = csalt_resource_decorator_logger_errors(
		(csalt_resource *)&stub_fail,
		write_fd,
		messages
	);

	csalt_store *result = csalt_resource_init((csalt_resource *)&logger);
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
