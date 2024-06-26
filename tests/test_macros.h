#ifndef TEST_MACROS_H
#define TEST_MACROS_H

#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>

#include "csalt/stores.h"
#include "csalt/resources.h"

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1
#define EXIT_TEST_ERROR 99
#define EXIT_TEST_SKIPPED 77
#define print_error(...) do {	\
	fprintf(stderr, "%s:%d: ", __FILE__, __LINE__);	\
	fprintf(stderr, __VA_ARGS__);	\
	fprintf(stderr, "\n");	\
} while (0)

#define print_error_and_exit(...) do { \
	print_error(__VA_ARGS__); \
	abort(); \
} while (0)

// Code generation hell incoming
// I'm going to hell for this
#define ARG(arg) arg
#define ARGS(...) __VA_ARGS__

#define CREATE_IMPL_POINTER(ret, name, args) \
ret (*_##name##_impl)(args) = NULL

#define CALL_IMPL(name, args) \
_##name##_impl(args)

#define CREATE_IMPL_WRAPPER(ret, name, args, call) \
ret name(args) \
{ \
	return call; \
}

#define INIT_IMPL(ret, name, args, call_args) \
	CREATE_IMPL_POINTER(ret, name, ARGS(args)); \
	CREATE_IMPL_WRAPPER(ret, name, ARGS(args), CALL_IMPL(name, ARGS(call_args)))

#define SET_IMPL(name, function) \
_##name##_impl = function


struct csalt_static_store_stub {
	struct csalt_static_store_interface *vtable;
	ssize_t last_read;
	ssize_t last_write;
	ssize_t split_begin;
	ssize_t split_end;
	ssize_t size;
};

struct csalt_dynamic_store_stub {
	struct csalt_static_store_stub parent;
};

ssize_t csalt_static_store_stub_read(csalt_static_store *store, void *data, ssize_t amount)
{
	(void)data;
	struct csalt_static_store_stub *stub = (void *)store;
	stub->last_read = csalt_min(amount, stub->size);
	return stub->last_read;
}

ssize_t csalt_static_store_stub_write(csalt_static_store *store, const void *data, ssize_t amount)
{
	(void)data;
	struct csalt_static_store_stub *stub = (void *)store;
	stub->last_write = csalt_min(amount, stub->size);
	return stub->last_write;
}

int csalt_static_store_stub_split(
	csalt_static_store *store,
	ssize_t begin,
	ssize_t end,
	csalt_static_store_block_fn *block,
	void *data
)
{
	struct csalt_static_store_stub *stub = (void *)store;
	stub->split_begin = begin;
	stub->split_end = end;
	int result = block(store, data);
	stub->split_begin = 0;
	stub->split_end = 0;
	return result;
}

ssize_t csalt_dynamic_store_stub_size(csalt_store *store)
{
	struct csalt_dynamic_store_stub *stub = (void *)store;
	return stub->parent.size;
}

ssize_t csalt_dynamic_store_stub_resize(
	csalt_store *store,
	ssize_t new_size
)
{
	struct csalt_dynamic_store_stub *stub = (void *)store;
	stub->parent.size = new_size;
	return new_size;
}

struct csalt_dynamic_store_interface csalt_dynamic_store_stub_interface = {
	{
		csalt_static_store_stub_read,
		csalt_static_store_stub_write,
		csalt_static_store_stub_split,
	},
	csalt_dynamic_store_stub_size,
	csalt_dynamic_store_stub_resize,
};

struct csalt_static_store_stub csalt_static_store_stub(ssize_t size)
{
	return (struct csalt_static_store_stub) {
		.vtable = &csalt_dynamic_store_stub_interface.parent,
		.size = size,
	};
}

struct csalt_dynamic_store_stub csalt_dynamic_store_stub(ssize_t size)
{
	return (struct csalt_dynamic_store_stub) {
		csalt_static_store_stub(size),
	};
}

ssize_t csalt_static_store_stub_error_read(csalt_static_store *_, void *__, ssize_t ___)
{
	(void)_;
	(void)__;
	(void)___;
	return -1;
}

ssize_t csalt_static_store_stub_error_write(csalt_static_store *_, const void *__, ssize_t ___)
{
	(void)_;
	(void)__;
	(void)___;
	return -1;
}

ssize_t csalt_dynamic_store_stub_error_size(csalt_store *_)
{
	(void)_;
	return 0;
}

ssize_t csalt_dynamic_store_stub_error_resize(
	csalt_store *store,
	ssize_t __
)
{
	(void)__;
	struct csalt_dynamic_store_stub *stub = (void *)store;
	return stub->parent.size;
}

struct csalt_dynamic_store_interface csalt_dynamic_store_stub_error_implementation = {
	{
		csalt_static_store_stub_error_read,
		csalt_static_store_stub_error_write,
		csalt_static_store_stub_split,
	},
	csalt_dynamic_store_stub_error_size,
	csalt_dynamic_store_stub_error_resize,
};

struct csalt_static_store_stub csalt_static_store_stub_error()
{
	struct csalt_static_store_stub result = {
		.vtable = &csalt_dynamic_store_stub_error_implementation.parent,
	};
	return result;
}

struct csalt_dynamic_store_stub csalt_dynamic_store_stub_error()
{
	return (struct csalt_dynamic_store_stub) {
		csalt_static_store_stub_error(),
	};
}

ssize_t csalt_static_store_stub_zero_read(csalt_static_store *_, void *__, ssize_t ___)
{
	(void)_;
	(void)__;
	(void)___;
	return 0;
}

ssize_t csalt_static_store_stub_zero_write(csalt_static_store *_, const void *__, ssize_t ___)
{
	(void)_;
	(void)__;
	(void)___;
	return 0;
}

struct csalt_dynamic_store_interface csalt_dynamic_store_stub_zero_implementation = {
	{
		csalt_static_store_stub_zero_read,
		csalt_static_store_stub_zero_write,
		csalt_static_store_stub_split,
	},
	csalt_dynamic_store_stub_size,
	csalt_dynamic_store_stub_resize,
};

struct csalt_static_store_stub csalt_static_store_stub_zero()
{
	return (struct csalt_static_store_stub) {
		.vtable = &csalt_dynamic_store_stub_zero_implementation.parent,
	};
}

struct csalt_dynamic_store_stub csalt_dynamic_store_stub_zero()
{
	return (struct csalt_dynamic_store_stub) {
		csalt_static_store_stub_zero(),
	};
}


struct csalt_resource_stub {
	const struct csalt_dynamic_resource_interface *vtable;
	int init_called;
	int deinit_called;
	int should_fail;

	struct csalt_dynamic_store_stub return_value;
};

csalt_store *csalt_resource_stub_init(csalt_resource *resource)
{
	struct csalt_resource_stub *stub = (void *)resource;

	stub->init_called++;
	if (stub->should_fail)
		return NULL;
	return (csalt_store *)&stub->return_value;
}

void csalt_resource_stub_deinit(csalt_resource *resource)
{
	struct csalt_resource_stub *stub = (void *)resource;

	stub->deinit_called++;
}

static struct csalt_dynamic_resource_interface stub_resource_impl = {
	csalt_resource_stub_init,
	csalt_resource_stub_deinit,
};

struct csalt_resource_stub csalt_resource_stub(int should_fail)
{
	return (struct csalt_resource_stub) {
		&stub_resource_impl,
		0,
		0,
		should_fail,
		csalt_dynamic_store_stub(0),
	};
}

#endif // TEST_MACROS_H
