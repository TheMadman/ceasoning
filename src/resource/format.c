#include "csalt/resource/format.h"

#include <stdarg.h>
#include <stdio.h>

typedef struct csalt_resource_format format_t;

csalt_store *csalt_resource_format_init(csalt_resource *resource)
{
	format_t *format = (void*)resource;
	va_list copy;
	va_copy(copy, format->args);
	const int needed = vsnprintf(NULL, 0, format->format, copy) + 1;
	va_end(copy);

	if (needed <= 0)
		return NULL;

	format->heap = csalt_resource_heap(needed);
	struct csalt_store_heap *attempt = (void*)csalt_resource_init((void*)&format->heap);
	if (!attempt)
		return NULL;

	va_copy(copy, format->args);
	const int success = vsnprintf(attempt->begin, (unsigned)needed, format->format, copy);
	va_end(copy);
	if (success <= 0) {
		csalt_resource_deinit((void*)attempt);
		return NULL;
	}

	return (void*)attempt;
}

void csalt_resource_format_deinit(csalt_resource *resource)
{
	format_t *format = (void*)resource;
	csalt_resource_deinit((void*)&format->heap);
	va_end(format->args);
}

static const struct csalt_dynamic_resource_interface impl = {
	csalt_resource_format_init,
	csalt_resource_format_deinit,
};

struct csalt_resource_format csalt_resource_format_vargs(
	const char *format,
	va_list args
)
{
	format_t result = {
		.vtable = &impl,
		.format = format,
	};
	va_copy(result.args, args);
	return result;
}


struct csalt_resource_format csalt_resource_format(
	const char *format,
	...
)
{
	va_list args;
	va_start(args, format);
	return csalt_resource_format_vargs(format, args);
}

int csalt_use_format(
	csalt_store_block_fn *block,
	void *param,
	const char *format,
	...
)
{
	va_list args;
	va_start(args, format);
	format_t result = csalt_resource_format_vargs(format, args);
	return csalt_resource_use((void*)&result, block, param);
}

