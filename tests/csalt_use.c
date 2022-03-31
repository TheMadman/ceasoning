#include "csalt/resources.h"

#include "test_macros.h"

static char use_store(csalt_store *store, char a, char b, char c)
{
	return a | b | c;
}

static csalt_store *init(csalt_resource *resource)
{
	return (void *)1;
}

static void deinit(csalt_resource *resource)
{
	(void)resource;
}

static const struct csalt_resource_interface test_resource_interface = {
	init,
	deinit,
};
static const csalt_resource test_resource = &test_resource_interface;

int main()
{
	char result = 0, a = 1, b = 2, c = 4;
	CSALT_USE(result, &test_resource, use_store, a, b, c);
	if (result != (a | b | c)) {
		print_error("Result was unexpected value: %d", result);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
