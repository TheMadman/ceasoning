#include <csalt/resources.h>

#include "test_macros.h"

int use_called = 0;

int use(csalt_store *resource, void *out)
{
	use_called = 1;
	return 0;
}

int main()
{
	struct csalt_heap
		heap1 = csalt_heap(1),
		heap2 = csalt_heap(-1);

	csalt_resource *array[] = {
		csalt_resource(&heap1),
		csalt_resource(&heap2),
	};

	csalt_resource_initialized *buffer[2] = { 0 };
	struct csalt_resource_list list = csalt_resource_list_array(array, buffer);

	csalt_resource_use(csalt_resource(&list), use, 0);
	if (use_called) {
		print_error("use() was called when it shouldn't have been");
		return EXIT_TEST_FAILURE;
	}

	struct csalt_heap
		heap3 = csalt_heap(4),
		heap4 = csalt_heap(8);

	array[0] = csalt_resource(&heap3);
	array[1] = csalt_resource(&heap4);

	// list = csalt_resource_list_array(array, buffer);

	csalt_resource_use(csalt_resource(&list), use, 0);
	if (!use_called) {
		print_error("use() was not called when it should have been");
		return EXIT_TEST_FAILURE;
	}

	return EXIT_SUCCESS;
}

