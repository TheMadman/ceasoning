#include "csalt/stores.h"

#include "test_macros.h"

#include <string.h>

int main()
{
	int a = 0, b = 1;

	struct csalt_memory A = csalt_store_memory_pointer(&a),
			    B = csalt_store_memory_pointer(&b);

	csalt_store_transfer((csalt_store *)&A, (csalt_store *)&B, sizeof(a));

	if (b) {
		print_error("int b still had non-zero value: %d", b);
		return EXIT_TEST_FAILURE;
	}

	// test huge values
	size_t arrsize = 1 << 29;
	char c[arrsize], d[arrsize];
	memset(c, 1, arrsize);
	memset(d, 0, arrsize);

	struct csalt_memory C = csalt_store_memory_array(c),
			    D = csalt_store_memory_array(d);

	csalt_store_transfer((csalt_store *)&C, (csalt_store *)&D, arrsize);

	for (char *test = d; test < &d[arrsize]; test++) {
		if (*test != 1) {
			print_error("Byte %ld doesn't match expected value: %c", test - d, *d);
			return EXIT_TEST_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}
