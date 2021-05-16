#include <csalt/resources.h>

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

int bytewise_copy(csalt_store *, void *);

char usage_format[] = "Usage: %s file-from file-to";

int main(int argc, char **argv)
{
	if (argc < 3) {
		fprintf(stderr, usage_format, argv[0]);
		return EXIT_FAILURE;
	}

	/*
	 * Create our files, one for reading and one for writing.
	 * 
	 * These calls do not attempt to open the files; that is only
	 * done when they are passed to csalt_resource_use.
	 */
	struct csalt_resource_file
		input = csalt_resource_file(argv[1], O_RDONLY),
		output = csalt_resource_create_file(argv[2], O_WRONLY, 0644);

	/*
	 * Prepare an array to use for our list resource.
	 */
	csalt_resource *resource_array[] = {
		csalt_resource(&input),
		csalt_resource(&output),
	};

	csalt_resource_initialized *buffer[2] = { 0 };

	/*
	 * The csalt_resource_list allows us to treat multiple resources
	 * as a single resource.
	 */
	struct csalt_resource_list list = csalt_resource_list_array(resource_array, buffer);

	/*
	 * When `list` is initialized by csalt_resource_use, it attempts to
	 * initialize the first resource. If that fails, it returns without
	 * calling bytewise_copy. If it succeeds, it moves onto the second resource.
	 *
	 * If the second resource fails, it deinitializes the first resource and
	 * returns without calling bytewise_copy.
	 *
	 * Only if both resources can be correctly initialized does the bytewise_copy
	 * function run.
	 *
	 * csalt_resource_use returns -1 when the resource fails to initialize;
	 * otherwise, it returns the return value of the function passed, in this
	 * case, bytewise_copy.
	 */
	int result = 0;
	if ((result = csalt_resource_use(csalt_resource(&list), bytewise_copy, 0)))
		fprintf(stderr, "Error: %s\n", strerror(errno));
	return result;
}

int bytewise_copy(csalt_store *resource, void *_)
{
	/*
	 * The second argument, the void*, can act as an in/out parameter for this
	 * function.
	 * In this case, we just want to copy the contents of one file into another,
	 * then exit, so we pass 0 to csalt_resource_use and just don't use the second
	 * argument here.
	 */

	struct csalt_store_list *list = (struct csalt_store_list *)resource;

	/*
	 * Resources implement the store interface, and can be safely cast from
	 * resources to stores.
	 *
	 * While in this case, we know the exact types of these variables
	 * (csalt_resource_file), it is actually more convenient for us to
	 * use them as generic stores, and it makes our function more flexible,
	 * to boot.
	 */
	csalt_store
		*input = (csalt_store *)csalt_store_list_get(list, 0),
		*output = (csalt_store *)csalt_store_list_get(list, 1);

	ssize_t input_size = csalt_store_size(input);
	struct csalt_progress progress = csalt_progress(input_size);

	/*
	 * The ceasoning library is implemented as a non-blocking library by
	 * default. Blocking behaviour can be achieved through a simple spinlock.
	 */
	for (
		ssize_t transferred = 0;
		transferred < input_size;
		transferred = csalt_store_transfer(&progress, output, input, 0)
	) {
		/*
		 * Always do error checking!
		 */
		if (transferred < 0)
			return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

