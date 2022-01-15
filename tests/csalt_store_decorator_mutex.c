#include <csalt/stores.h>

#include "test_macros.h"

#include <csalt/platform/threads.h>

csalt_mutex mutex;

ssize_t csalt_store_mutex_stub_read(
	csalt_store *store,
	void *buffer,
	size_t amount
)
{
	if (!csalt_mutex_trylock(&mutex))
		return -1;
	return amount;
}

ssize_t csalt_store_mutex_stub_write(
	csalt_store *store,
	const void *buffer,
	size_t amount
)
{
	if (!csalt_mutex_trylock(&mutex))
		return -1;
	return amount;
}

int csalt_store_mutex_stub_split(
	csalt_store *store,
	size_t begin,
	size_t end,
	csalt_store_block_fn *block,
	void *param
)
{
	if (!csalt_mutex_trylock(&mutex))
		return -1;
	return 0;
}

struct csalt_store_interface csalt_store_mutex_stub_implementation = {
	csalt_store_mutex_stub_read,
	csalt_store_mutex_stub_write,
	0,
	csalt_store_mutex_stub_split,
};

int main()
{
	{
		int mutex_result = csalt_mutex_init(&mutex, 0);
		if (mutex_result) {
			print_error("Error initializing mutex");
			return EXIT_TEST_ERROR;
		}

		const struct csalt_store_interface
			*mutex_stub = &csalt_store_mutex_stub_implementation;

		struct csalt_store_decorator_mutex
			decorator = csalt_store_decorator_mutex(
				csalt_store(&mutex_stub),
				&mutex
			);

		if (csalt_store_read(csalt_store(&decorator), 0, 0)) {
			print_error("Mutex was available when it should have been locked");
			return EXIT_FAILURE;
		}

		csalt_mutex_deinit(&mutex);
	}

	{
		int mutex_result = csalt_mutex_init(&mutex, 0);
		if (mutex_result) {
			print_error("Error initializing mutex");
			return EXIT_TEST_ERROR;
		}

		const struct csalt_store_interface
			*mutex_stub = &csalt_store_mutex_stub_implementation;

		struct csalt_store_decorator_mutex
			decorator = csalt_store_decorator_mutex(
				csalt_store(&mutex_stub),
				&mutex
			);

		if (csalt_store_write(csalt_store(&decorator), 0, 0)) {
			print_error("Mutex was available when it should have been locked");
			return EXIT_FAILURE;
		}

		csalt_mutex_deinit(&mutex);
	}

	{
		int mutex_result = csalt_mutex_init(&mutex, 0);
		if (mutex_result) {
			print_error("Error initializing mutex");
			return EXIT_TEST_ERROR;
		}

		const struct csalt_store_interface
			*mutex_stub = &csalt_store_mutex_stub_implementation;

		struct csalt_store_decorator_mutex
			decorator = csalt_store_decorator_mutex(
				csalt_store(&mutex_stub),
				&mutex
			);

		if (csalt_store_split(csalt_store(&decorator), 0, 0, 0, 0)) {
			print_error("Mutex was available when it should have been locked");
			return EXIT_FAILURE;
		}

		csalt_mutex_deinit(&mutex);
	}

	return EXIT_SUCCESS;
}