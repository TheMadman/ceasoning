#ifndef SALTRESOURCES_H
#define SALTRESOURCES_H

#include <stddef.h>

/**
 * \file
 * \brief Provides an interface for resources with lifetimes.
 *
 * This file provides both a common interface for resources which
 * require clean-up; a function to manage a resource automatically;
 * and some resource structs for common program resources like
 * heap memory and file descriptors.
 */

#ifdef __cplusplus
extern "C" {
#endif

struct csalt_resource_interface;

/**
 * To create custom structs which can manage resources,
 * use a struct csalt_resource_interface* as the first
 * member.
 *
 * Functions operating on resources take a csalt_resource
 * pointer; casting a pointer to your custom struct to
 * a (csalt_resource *) will allow you to use it in those
 * functions.
 */
typedef struct csalt_resource_interface *csalt_resource;

/**
 * Function type for initializing the test on first use,
 * allows lazy evaluation of resources
 */
typedef void csalt_resource_init_fn(csalt_resource *resource);

/**
 * function type for fetching a pointer to the resulting
 * resource.
 */
typedef void *csalt_resource_pointer_fn(csalt_resource *resource);

/**
 * Function type for checking if resource allocation
 * was successful
 */
typedef char csalt_resource_valid_fn(void *);

/**
 * Function type for cleaning up a resource.
 */
typedef void csalt_deinit_fn(void *);

/**
 * Interface definition for managed resources.
 * Structs with a pointer-to-resource-interface
 * as their first member can be passed to resource
 * functions with a simple cast.
 *
 * This struct should not be instantiated instantly,
 * but instead be a member of a struct which is
 * itself set up with a function.
 */
struct csalt_resource_interface {
	csalt_resource_init_fn *init;
	csalt_resource_pointer_fn *get_pointer;
	csalt_resource_valid_fn *valid;
	csalt_deinit_fn *deinit;
};

/**
 * Initializes a resource
 */
void csalt_resource_init(csalt_resource *);

/**
 * Returns the resource pointer from a given resource.
 */
void *csalt_resource_pointer(csalt_resource *);

/**
 * Returns whether resource creation was successful.
 */
char csalt_resource_valid(csalt_resource *);

/**
 * Cleans up the resource. The resource should
 * be considered invalid after run.
 */
void csalt_resource_deinit(csalt_resource *);

/**
 * Function signature for blocks to pass to csalt_use.
 * The function should expect a pointer-to-resource as the
 * only argument, and return any result you wish to pass on,
 * or a null pointer.
 */
typedef void *csalt_resource_block(void *);

/**
 * Takes a pointer to resource struct and a code block,
 * passes the resource to the code block, cleans up the
 * resource and finally, returns the result of code_block.
 *
 * This function also checks if resource allocation was
 * successful and exits immediately when it failed.
 * Checking validity with csalt_resource_valid can be skipped
 * if the function passed in code_block returns an error value
 * on failure.
 *
 * In the following example, we create a function which reads
 * a file's contents from a file descriptor into heap memory
 * and prints it. Since we don't use a resource for the heap
 * memory, we have to manually free it where-ever an error can
 * occur, and finally after successfully using it. However,
 * the csalt_use function manages the file descriptor, which
 * is checked before use and closed after read_content is
 * done.
 * \code
 * #include <sys/types.h>
 * #include <unistd.h>
 * #include <stdio.h>
 *
 * void *read_content(void *fd_pointer)
 * {
 * 	int fd = *fd_pointer;
 * 	off_t size = lseek(fd, 0, SEEK_END);
 * 	lseek(fd, 0, SEEK_SET);
 * 	void *result = malloc(size);
 * 	if (!result)
 * 		return 0;
 *
 * 	ssize_t amount_read = read(fd, result, size);
 * 	if (amount_read < 0) {
 * 		free(result);
 * 		return 0;
 * 	}
 * 	return result;
 * }
 *
 * int main(int argc, char **argv)
 * {
 * 	csalt_file file = csalt_file_init("test.txt", "rw");
 *	char *content = csalt_use((csalt_resource *)&file, read_content);
 *	if (content) {
 *		puts(content);
 *		free(content);
 *		return 0;
 *	} else {
 *		fputs(stderr, "Error reading file");
 *		return 1;
 *	}
 * }
 *
 * \endcode
 */
void *csalt_use(csalt_resource *resource, csalt_resource_block *code_block);

/**
 * Represents a heap memory resource.
 *
 * Avoid using or modifying the members directly - simple code should
 * create this struct with csalt_memory_init and pass it to csalt_use,
 * or use it as a member for another resource.
 */
typedef struct {
	const struct csalt_resource_interface * const vtable;
	size_t size;
	void *resource_pointer;
} csalt_memory;

/**
 * Initializes a csalt_memory resource. Uses malloc internally -
 * memory is allocated but not initialized.
 */
csalt_memory csalt_memory_make(size_t size);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // SALTRESOURCES_H
