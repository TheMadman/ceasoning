#include "csalt/stores.h"
#include <stdio.h>

#include "test_macros.h"

int read_called = 0;
int write_called = 0;
int split_begin = 0;
int split_end = 100;

int split_block(csalt_store *store, void *_);

struct test_struct {
	struct csalt_store_interface *implementation;
	size_t begin;
	size_t end;
};

ssize_t test_read(const csalt_store *store, void *buffer, size_t size)
{
	(void)store;
	(void)buffer;
	(void)size;
	read_called++;
	return 0;
}

ssize_t test_write(csalt_store *store, const void *buffer, size_t size)
{
	(void)store;
	(void)buffer;
	(void)size;
	write_called++;
	return 0;
}

size_t test_size(const csalt_store *store)
{
	const struct test_struct *data = (struct test_struct *)store;
	return data->end - data->begin;
}

int test_split(
	csalt_store *store,
	size_t begin,
	size_t end,
	csalt_store_block_fn *block,
	void *_
)
{
	struct test_struct *data = (struct test_struct *)store;
	struct test_struct result = {
		data->implementation,
		data->begin + begin,
		data->begin + end
	};
	return block((csalt_store *)&result, data);
}

struct csalt_store_interface test_implementation = {
	test_read,
	test_write,
	test_size,
	test_split,
};

int split_block(csalt_store *store, void *_)
{
	(void)_;
	struct test_struct *impl = (struct test_struct *)store;
	split_begin = impl->begin;
	split_end = impl->end;
	return 0;
}

int main()
{
	struct test_struct data = {
		&test_implementation,
		split_begin,
		split_end,
	};

	csalt_store_read((csalt_store *)&data, 0, 0);
	if (!read_called) {
		print_error("Read call failed");
		return EXIT_FAILURE;
	}

	csalt_store_write((csalt_store *)&data, 0, 0);
	if (!write_called) {
		print_error("Write call failed");
		return EXIT_FAILURE;
	}

	csalt_store_split(
		(csalt_store *)&data,
		0,
		50,
		split_block,
		0
	);

	if (split_begin != 0 || split_end != 50) {
		print_error("Split failed, split_begin: %d split_end: %d", split_begin, split_end);
		return EXIT_FAILURE;
	}

	csalt_store_split(
		(csalt_store *)&data,
		10,
		20,
		split_block,
		0
	);

	if (split_begin != 10 || split_end != 20) {
		print_error("Split failed, split_begin: %d split_end: %d", split_begin, split_end);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

