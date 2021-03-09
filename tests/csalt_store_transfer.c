#include "csalt/stores.h"

#include "test_macros.h"
#include "csalt/util.h"

#include <string.h>
#include <stdlib.h>

#define ARRSIZE 1 << 20
char c[ARRSIZE], d[ARRSIZE];
int transfer_complete_big_called = 0;

void transfer_complete_small(csalt_store *destination)
{
	struct csalt_memory *memory = castto(memory, destination);
	int *b = csalt_store_memory_raw(memory);

	if (*b) {
		print_error("b still contained non-zero value: %d\n", *b);
		exit(EXIT_TEST_FAILURE);
	}
}

void transfer_complete_big(csalt_store *destination)
{
	(void)destination;
	transfer_complete_big_called++;
}

int main()
{
	int a = 0, b = 1;

	struct csalt_memory A = csalt_store_memory_pointer(&a),
			    B = csalt_store_memory_pointer(&b);

	struct csalt_transfer transfer = csalt_transfer(sizeof(a));
	csalt_store_transfer(
		&transfer,
		castto(csalt_store *, &B),
		castto(csalt_store *, &A),
		transfer_complete_small
	);

	// test larger-than-page values
	memset(c, 1, ARRSIZE);
	memset(d, 0, ARRSIZE);

	struct csalt_memory C = csalt_store_memory_array(c),
			    D = csalt_store_memory_array(d);

	transfer = csalt_transfer(ARRSIZE);
	ssize_t transfer_amount = 0;
	while ((transfer_amount = csalt_store_transfer(
		&transfer,
		castto(csalt_store *, &D),
		castto(csalt_store *, &C),
		transfer_complete_big
	)) < ARRSIZE) {
		switch (transfer_amount) {
			case -1:
				print_error("Error code returned");
				return EXIT_TEST_FAILURE;
			case 0:
				print_error("Zero bytes transfered");
				return EXIT_TEST_FAILURE;
			case ARRSIZE:
				break;
		}
	}

	if (!transfer_complete_big_called) {
		print_error(
			"Transfer completed, but "
			"callback wasn't called"
		);
		return EXIT_TEST_FAILURE;
	}

	for (char *test = d; test < &d[ARRSIZE]; test++) {
		if (*test != 1) {
			print_error("Byte %ld doesn't match expected value: %c", test - d, *test);
			return EXIT_TEST_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}
