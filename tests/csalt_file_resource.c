#include <csalt/fileresource.h>

#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

#include "test_macros.h"

// Unlinkly to be duplicated by a human by accident
#define FILENAME "cay+5wGBIpGOM6DXxvAFDmqW"

int test_write_called = 0;
int test_read_called = 0;

struct csalt_heap test_write(csalt_resource *resource)
{
	test_write_called = 1;
	struct csalt_resource_file *file = castto(file, resource);
	int a = 1;
	struct csalt_memory A = csalt_store_memory_pointer(&a);

	ssize_t amount_written = csalt_store_transfer(
		castto(csalt_store *, resource),
		castto(csalt_store *, &A),
		sizeof(a)
	);
	if (amount_written != sizeof(a)) {
		print_error(
			"Unexpected number of bytes written, "
			"expected: %ld actual: %ld",
			sizeof(a),
			amount_written
		);

		// this whole function is skipped if the fd is invalid
		unlink(file->filename);
		exit(EXIT_TEST_FAILURE);
	}

	return csalt_null_heap;
}

struct csalt_heap test_read(csalt_resource *resource)
{
	test_read_called = 1;
	struct csalt_resource_file *file = castto(file, resource);
	int a = 0;
	struct csalt_memory A = csalt_store_memory_pointer(&a);

	ssize_t amount_read = csalt_store_transfer(
		castto(csalt_store *, &A),
		castto(csalt_store *, resource),
	       	sizeof(a)
	);
	if (amount_read != sizeof(a)) {
		print_error(
			"Unexpected number of bytes read, "
			"expected: %ld actual: %ld",
			sizeof(a),
			amount_read
		);
		unlink(file->filename);
		exit(EXIT_TEST_FAILURE);
	}
	if (!a) {
		print_error(
			"Read value didn't match expected value, "
			"expected: %d actual: %d",
			1,
			a
		);
		unlink(file->filename);
		exit(EXIT_TEST_FAILURE);
	}
	return csalt_null_heap;
}

int main()
{
	// first constructor -- creates file if not exists
	struct csalt_resource_file file = csalt_resource_create_file("./" FILENAME, O_RDWR, 0600);
	csalt_resource_use(castto(csalt_resource *, &file), test_write);
	if (!test_write_called) {
		print_error("Write wasn't called when it should have been");
		unlink("./" FILENAME);
		return EXIT_TEST_FAILURE;
	}

	// second constructor -- file must exist before init
	struct csalt_resource_file file2 = csalt_resource_file("./" FILENAME, O_RDONLY);
	csalt_resource_use(castto(csalt_resource *, &file2), test_read);
	if (!test_read_called) {
		print_error("Read wasn't called when it should have been");
		unlink("./" FILENAME);
		return EXIT_TEST_FAILURE;
	}

	unlink("./" FILENAME);

	return EXIT_SUCCESS;
}

