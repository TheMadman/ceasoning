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

struct salt_resource_interface;

/**
 * To create custom structs which can manage resources,
 * use a struct salt_resource_interface* as the first
 * member.
 *
 * Functions operating on resources take a salt_resource
 * pointer; casting a pointer to your custom struct to
 * a (salt_resource *) will allow you to use it in those
 * functions.
 */
typedef struct salt_resource_interface *salt_resource;

/**
 * function type for fetching a pointer to the resulting
 * resource.
 */
typedef void *salt_resource_pointer_fn(salt_resource *resource);

/**
 * Function type for checking if resource allocation
 * was successful
 */
typedef char salt_resource_valid_fn(void *);

/**
 * Function type for cleaning up a resource.
 */
typedef void salt_deinit_fn(void *);

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
struct salt_resource_interface {
	salt_resource_pointer_fn *get_pointer;
	salt_resource_valid_fn *valid;
	salt_deinit_fn *deinit;
};

/**
 * Returns the resource pointer from a given resource.
 */
void *salt_resource_pointer(salt_resource *);

/**
 * Returns whether resource creation was successful.
 */
char salt_resource_valid(salt_resource *);

/**
 * Cleans up the resource. The resource should
 * be considered invalid after run.
 */
void salt_resource_deinit(salt_resource *);

/**
 * Function signature for blocks to pass to salt_use.
 * The function should expect a pointer-to-resource as the
 * only argument, and return any result you wish to pass on,
 * or a null pointer.
 */
typedef void *salt_resource_block(void *);

/**
 * Takes a pointer to resource struct and a code block,
 * passes the resource to the code block, cleans up the
 * resource and finally, returns the result of code_block.
 *
 * This function also checks if resource allocation was
 * successful and exits immediately when it failed.
 * Checking validity with salt_resource_valid can be skipped
 * if the function passed in code_block returns an error value
 * on failure.
 *
 * In the following example, we create a function which reads
 * a file's contents from a file descriptor into heap memory
 * and prints it. Since we don't use a resource for the heap
 * memory, we have to manually free it where-ever an error can
 * occur, and finally after successfully using it. However,
 * the salt_use function manages the file descriptor, which
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
 * 	salt_file file = salt_file_init("test.txt", "rw");
 *	char *content = salt_use((salt_resource *)&file, read_content);
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
void *salt_use(salt_resource *resource, salt_resource_block *code_block);

/**
 * Represents a heap memory resource.
 *
 * Avoid using or modifying the members directly - simple code should
 * create this struct with salt_memory_init and pass it to salt_use,
 * or use it as a member for another resource.
 */
typedef struct {
	const struct salt_resource_interface * const vtable;
	void *resource_pointer;
} salt_memory;

/**
 * Initializes a salt_memory resource. Uses malloc internally -
 * memory is allocated but not initialized.
 */
salt_memory salt_memory_init(size_t size);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // SALTRESOURCES_H
