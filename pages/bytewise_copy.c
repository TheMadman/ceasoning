#include <csalt/resources.h>
#include <stdio.h>

int bytewise_copy(csalt_store *store, void *);

int main(int argc, char *argv[])
{
	/*
	 * Check if we have enough arguments
	 */
	if (argc < 3) {
		printf("Usage: %s <input-file> <output-file>\n", argv[0]);
		return 1;
	}

	/*
	 * Create two file resources - one for input, one for output.
	 * These calls don't attempt to open the files yet, that happens
	 * later.
	 */
	struct csalt_resource_file
		input = csalt_resource_file(argv[1], O_RDONLY),
		output = csalt_resource_create_file(argv[2], O_WRONLY, 0644);

	/*
	 * Add them to an array - this will be used to initialize a list
	 */
	csalt_resource *resources[] = {
		csalt_resource(&input),
		csalt_resource(&output),
	};

	/*
	 * Create an array of resource pairs, which will store our list
	 */
	struct csalt_resource_pair list[arrlength(resources)] = { 0 };

	/*
	 * This macro wraps a function, taking two in-place arrays and
	 * initializing list based on resources
	 */
	csalt_resource_pair_list(resources, list);

	/*
	 * This is where the magic happens.
	 *
	 * This function attempts to initialize the first resource in
	 * our list. If that fails, it returns -1. Otherwise, it attempts
	 * the second resource in the list.
	 *
	 * If the second resource fails, the first resource is correctly
	 * cleaned up for us, then the function returns -1.
	 *
	 * Only if both resources open successfully do we run bytewise_copy.
	 * Then, we return the return value of bytewise_copy.
	 */
	int result = csalt_resource_use(
		csalt_resource(&list),
		bytewise_copy,
		0
	);

	if (result < 0)
		perror("Error opening file");
	return result;
}

int bytewise_copy(csalt_store *store, void *_)
{
	(void)_;
	/*
	 * A resource-pair, when initialized, returns a store-pair.
	 * We can use this to get our individual file stores to begin
	 * transfering data.
	 */
	struct csalt_store_pair *pairs = (struct csalt_store_pair *)store;

	/*
	 * csalt_store_pair_list_get is a convenience function around
	 * pairs that are arranged like a linked-list - which is what
	 * csalt_resource_pair_list does.
	 *
	 * Even though, in this case, we know that we're operating on
	 * the return values of csalt_resource_file, we only need the
	 * generic csalt_store interface - this also makes this function
	 * more generic, allowing it to accept ANY csalt_store_pair of
	 * two stores.
	 */
	csalt_store
		*first = csalt_store_pair_list_get(pairs, 0),
		*second = csalt_store_pair_list_get(pairs, 1);

	/*
	 * csalt_progress is a simple struct storing how much data
	 * has been transferred, vs. how much we expected.
	 */
	struct csalt_progress
		progress = csalt_progress(csalt_store_size(first));

	/*
	 * We want blocking behaviour for this application, which
	 * is achieved via a simple spin-looop.
	 *
	 * The ceasoning library is non-blocking by default, which
	 * allows the construction of composite types that can implement
	 * algorithms generically.
	 */
	while (!csalt_progress_complete(&progress))
		/*
		 * Non-blocking file descriptor types will return 0
		 * when there's no data to read, and -1 when the file
		 * descriptor is unavailable for reading/writing. That includes
		 * types representing network sockets, etc.
		 *
		 * This allows us to do one generic error check which works
		 * across all stores, without having to worry about
		 * EAGAIN or EWOULDBLOCK.
		 *
		 * csalt_store_transfer is a function accepting any two stores
		 * and attempting a non-blocking transfer between them. It
		 * returns the total amount copied across all calls, or -1 if
		 * there is an error.
		 */
		if (csalt_store_transfer(&progress, first, second, 0) < 0)
			return -1;
	return 0;
}

