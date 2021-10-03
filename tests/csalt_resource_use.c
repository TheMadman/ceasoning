#include "csalt/resources.h"

#include "test_macros.h"

int init_called = 0;
int read_called = 0;
int write_called = 0;
int block_called = 0;
int deinit_called = 0;

struct csalt_store_interface test_init_interface;
struct csalt_store_interface *test_init_ptr = &test_init_interface;

csalt_store *init(csalt_resource *_)
{
	init_called++;
	return &test_init_ptr;
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

int block(csalt_store *_, void *__)
{
	(void)_;
	(void)__;
	block_called++;
	return 0;
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
	init,
	deinit,
};

#define testvar(name) { name, #name }

int main()
{
	csalt_resource test = &test_interface;
	csalt_resource_use(&test, block, 0);

	struct test_var {
		int var;
		char *name;
	} variables_to_test[] = {
		testvar(init_called),
		testvar(block_called),
		testvar(deinit_called),

		// the following shouldn't have been called
		{ !read_called, "read_var" },
		{ !write_called, "write_called" },
	};

	for (
		struct test_var
			*current = variables_to_test,
			*end = &variables_to_test[arrlength(variables_to_test)];
		current < end;
		current++
	) {
		if (!current->var) {
			print_error("Unexpected value for %s", current->name);
			return EXIT_FAILURE;
		}
	}
	
	return 0;
}
