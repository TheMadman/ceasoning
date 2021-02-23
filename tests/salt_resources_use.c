#include "saltresources.h"

#define EXIT_SUCCESS 0
#define EXIT_TEST_FAILURE 99
#define EXIT_TEST_SKIPPED 77

int init_called = 0;
int pointer_called = 0;
int valid_called = 0;
int block_called = 0;
int deinit_called = 0;

void init(salt_resource *_)
{
	init_called++;
}

void *pointer(salt_resource *_)
{
	pointer_called++;
	return 0;
}

char valid(void *_)
{
	valid_called++;
	return 1;
}

void *block(void *_)
{
	block_called++;
	return 0;
}

void deinit(void *_)
{
	deinit_called++;
}

struct salt_resource_interface test_interface = {
	init,
	pointer,
	valid,
	deinit,
};

int main()
{
	salt_resource test = &test_interface;
	salt_use(&test, block);
	int result = (
		init_called &&
		pointer_called &&
		valid_called &&
		block_called &&
		deinit_called
	);

	return result ? 0 : EXIT_TEST_FAILURE;
}
