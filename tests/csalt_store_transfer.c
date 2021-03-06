#include "csalt/stores.h"

#include "test_macros.h"

#include <string.h>

#define ARRSIZE 1 << 20
char c[ARRSIZE], d[ARRSIZE];

int main()
{
	int a = 0, b = 1;

	struct csalt_memory A = csalt_store_memory_pointer(&a),
			    B = csalt_store_memory_pointer(&b);

	csalt_store_transfer((csalt_store *)&B, (csalt_store *)&A, sizeof(a));

	if (b) {
		print_error("int b still had non-zero value: %d", b);
		return EXIT_TEST_FAILURE;
	}

	// test larger-than-page values
	memset(c, 1, ARRSIZE);
	memset(d, 0, ARRSIZE);

	struct csalt_memory C = csalt_store_memory_array(c),
			    D = csalt_store_memory_array(d);

	csalt_store_transfer((csalt_store *)&D, (csalt_store *)&C, ARRSIZE);

	for (char *test = d; test < &d[ARRSIZE]; test++) {
		if (*test != 1) {
			print_error("Byte %ld doesn't match expected value: %c", test - d, *test);
			return EXIT_TEST_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}
