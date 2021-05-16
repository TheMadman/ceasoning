#include <csalt/stores.h>
#include <csalt/resources.h>
#include <stdlib.h>
#include <string.h>

#include "test_macros.h"

int split_called = 0;

int split(csalt_store *store, void *data)
{
	split_called = 1;

	(void)data;
	struct csalt_store_list *list = castto(list, store);

	csalt_store *zeroth = csalt_store_list_get(list, 0);
	csalt_store *first = csalt_store_list_get(list, 1);

	char buffer = 0;
	csalt_store_read(zeroth, &buffer, sizeof(buffer));
	if (buffer != 1) {
		print_error("csalt_a[10] unexpected value, expected: %d actual: %d",
			1,
			buffer
		);
		exit(EXIT_FAILURE);
	}

	csalt_store_read(first, &buffer, sizeof(buffer));
	if (buffer != 2) {
		print_error("csalt_a[10] unexpected value, expected: %d actual: %d",
			2,
			buffer
		);
		exit(EXIT_FAILURE);
	}
}

int main()
{
	char a[40] = { 0 };
	char b[40] = { 0 };

	struct csalt_memory csalt_a = csalt_store_memory_array(a);
	struct csalt_memory csalt_b = csalt_store_memory_array(b);

	csalt_store *stores[] = {
		csalt_store(&csalt_a),
		csalt_store(&csalt_b),
	};

	struct csalt_store_list list = csalt_store_list_array(stores);

	a[10] = 1;
	b[10] = 2;

	csalt_store_split(csalt_store(&list), 10, sizeof(a), split, 0);

	if (!split_called) {
		print_error("Split wasn't called");
		return EXIT_FAILURE;
	}

	const char write_value[] = "Hello, world!";

	csalt_store_write(csalt_store(&list), write_value, sizeof(write_value));

	if (strcmp(write_value, a)) {
		print_error("'a' contained unexpected value, expected: \"%s\" actual: \"%s\"",
			write_value,
			a
		);
		return EXIT_FAILURE;
	}
	
	if (strcmp(write_value, b)) {
		print_error("'b' contained unexpected value, expected: \"%s\" actual: \"%s\"",
			write_value,
			b
		);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
