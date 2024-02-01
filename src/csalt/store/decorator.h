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

#ifndef CSALT_STORES_DECORATOR_H
#define CSALT_STORES_DECORATOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "base.h"

/**
 * \file
 * \brief This module contains definitions for writing new decorators.
 *
 * The structs and methods in this file are not intended to be created
 * directly, but used inside your own struct definitions to simplify
 * creating decorators around store.
 *
 * The methods contained here are for including in custom virtual call tables,
 * and forward calls to the decorated store.
 */

/**
 * \brief A decorator to include as the first member in your struct.
 */
struct csalt_store_decorator {
	const struct csalt_dynamic_store_interface *vtable;
	union {
		csalt_static_store *decorated_static;
		csalt_store *decorated;
	};
};

ssize_t csalt_store_decorator_read(csalt_static_store *, void *, ssize_t);
ssize_t csalt_store_decorator_write(csalt_static_store *, const void *, ssize_t);
int csalt_store_decorator_split(
	csalt_static_store *,
	ssize_t,
	ssize_t,
	csalt_static_store_block_fn *,
	void *);
ssize_t csalt_store_decorator_size(csalt_store *);
ssize_t csalt_store_decorator_resize(csalt_store *, ssize_t);

#ifdef __cplusplus
} // extern "C"
#endif
#endif // CSALT_STORES_DECORATOR_H
