#include <csalt/stores.h>

#include "test_macros.h"

int main()
{
	struct csalt_store_stub stub = csalt_store_stub(1024);
	int data = 42;

	ssize_t write = csalt_write(&stub, data);

	if (write < 0) {
		print_error("Unexpected error writing");
		return EXIT_FAILURE;
	}

	if (write != sizeof(data)) {
		print_error("Unexpected csalt_write return value: %ld", write);
		return EXIT_FAILURE;
	}

	data = 0;

	ssize_t read = csalt_read(&stub, data);

	if (read < 0) {
		print_error("Unexpected error reading");
		return EXIT_FAILURE;
	}

	if (read != sizeof(data)) {
		print_error("Unexpected csalt_read return value: %ld", read);
		return EXIT_FAILURE;
	}
}
