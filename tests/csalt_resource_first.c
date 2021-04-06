#include <csalt/resources.h>

#include <stdlib.h>

#include "test_macros.h"

int use(csalt_resource *resource, csalt_store *output)
{
	struct csalt_resource_first *first = castto(first, resource);
	struct csalt_resource_list *list = castto(list, resource);

	csalt_resource
		*heap1 = csalt_resource_list_get(list, 0),
		*heap2 = csalt_resource_list_get(list, 1),
		*heap3 = csalt_resource_list_get(list, 2);

	if (csalt_resource_valid(heap1)) {
		print_error("heap1 was initialized when it shouldn't have been");
		exit(EXIT_TEST_FAILURE);
	}

	if (!csalt_resource_valid(heap2)) {
		print_error("heap2 wasn't initialized when it should have been");
		exit(EXIT_TEST_FAILURE);
	}

	if (csalt_resource_valid(heap3)) {
		print_error("heap3 was initialized when it shouldn't have been");
		exit(EXIT_TEST_FAILURE);
	}

	csalt_store_write(csalt_store(first), "a", 1);

	char buffer;
	int amount_read = csalt_store_read(csalt_store(heap1), &buffer, 1);
	if (amount_read >= 0) {
		print_error("Shouldn't have been able to read a value from heap1");
		exit(EXIT_TEST_FAILURE);
	}

	amount_read = csalt_store_read(csalt_store(heap2), &buffer, 1);
	if (amount_read != 1) {
		print_error("Should have been able to read a single byte from heap2");
		exit(EXIT_TEST_FAILURE);
	}

	if (buffer != 'a') {
		print_error("Unexpected value in buffer");
		exit(EXIT_TEST_FAILURE);
	}

	amount_read = csalt_store_read(csalt_store(heap3), &buffer, 1);
	if (amount_read >= 0) {
		print_error("Shouldn't have been able to read a value from heap3");
		exit(EXIT_TEST_FAILURE);
	}

	return 0;
}

int main()
{
	struct csalt_heap
		// heap with size -1 should fail
		heap1 = csalt_heap_lazy(-1),
		heap2 = csalt_heap_lazy(1),
		heap3 = csalt_heap_lazy(3);

	csalt_resource *array[] = {
		csalt_resource(&heap1),
		csalt_resource(&heap2),
		csalt_resource(&heap3),
	};

	struct csalt_resource_first first = csalt_resource_first_array(array);

	csalt_resource_use(csalt_resource(&first), use, 0);
	return EXIT_SUCCESS;
}
