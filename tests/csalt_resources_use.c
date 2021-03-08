#include "csalt/resources.h"

#include "test_macros.h"

int init_called = 0;
int read_called = 0;
int write_called = 0;
int valid_called = 0;
int block_called = 0;
int deinit_called = 0;

void init(csalt_resource *_)
{
	init_called++;
}

ssize_t test_read(const csalt_store *_, void *__, size_t size)
{
	(void)_;
	(void)__;
	(void)size;
	read_called++;
	return 0;
}

ssize_t test_write(csalt_store *_, const void *__, size_t size)
{
	(void)_;
	(void)__;
	(void)size;
	write_called++;
	return 0;
}

char valid(const csalt_resource *_)
{
	(void)_;
	valid_called++;
	return 1;
}

struct csalt_heap block(csalt_resource *_)
{
	(void)_;
	block_called++;
	return csalt_null_heap;
}

void deinit(csalt_resource *_)
{
	(void)_;
	deinit_called++;
}

const struct csalt_store_interface store_interface = {
	test_read,
	test_write,
};

struct csalt_resource_interface test_interface = {
	store_interface,
	init,
	valid,
	deinit,
};

int main()
{
	csalt_resource test = &test_interface;
	csalt_resource_use(&test, block);
	int result = (
		init_called &&
		valid_called &&
		block_called &&
		deinit_called &&

		// the following shouldn't have been called
		!read_called &&
		!write_called
	);

	return result ? 0 : EXIT_TEST_FAILURE;
}
