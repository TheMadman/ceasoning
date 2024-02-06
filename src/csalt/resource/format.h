/*
 * Ceasoning - Syntactic Sugar for Common C Tasks
 * Copyright (C) 2022   Marcus Harrison
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CSALT_RESOURCES_FORMAT_H
#define CSALT_RESOURCES_FORMAT_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \file
 * \brief Provides a run-time resource interface for format strings.
 */

#include "base.h"

#include <stdarg.h>

#include "csalt/store/base.h"
#include "heap.h"

/**
 * \extends csalt_resource
 * \brief Provides a way to allocate space for, format, then use
 * 	a single format string.
 */
struct csalt_resource_format {
	const struct csalt_dynamic_resource_interface *vtable;
	const char *format;
	va_list args;
	struct csalt_resource_heap heap;
};

csalt_store *csalt_resource_format_init(csalt_resource *);
void csalt_resource_format_deinit(csalt_resource *);

/**
 * \public \memberof csalt_resource_format
 * \brief Default constructor, taking a format string and variable
 * 	argument list.
 */
struct csalt_resource_format csalt_resource_format(
	const char *format,
	...);

/**
 * \public \memberof csalt_resource_format
 * \brief Alternative constructor, accepting a va_list argument.
 *
 * Note that args is passed in as a copy. You must call va_end on
 * your own copy of args.
 */
struct csalt_resource_format csalt_resource_format_vargs(
	const char *format,
	va_list args);

/**
 * \public \memberof csalt_resource_format
 * \brief This is a convenience for constructing and immediately
 * 	using a format string.
 *
 * \param block The function to call
 * \param param The additional parameter for the block
 * \param format The format string
 *
 * \returns -1 on failure, or the return value of block if it was
 * 	called.
 */
int csalt_use_format(
	csalt_store_block_fn *block,
	void *param,
	const char *format,
	...);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // CSALT_RESOURCES_FORMAT_H
