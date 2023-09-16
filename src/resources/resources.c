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

#include "csalt/resources.h"

#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>

#include "csalt/util.h"

// Virtual function calls

csalt_store *csalt_resource_init(csalt_resource *resource)
{
	return (*resource)->init(resource);
}

void csalt_resource_deinit(csalt_resource *resource)
{
	(*resource)->deinit(resource);
}

int csalt_resource_use(
	csalt_resource *resource,
	csalt_store_fn *code_block,
	void *data
)
{
	csalt_store *initialized = csalt_resource_init(resource);
	if (!initialized)
		return -1;
	int result = code_block(initialized, data);
	csalt_resource_deinit(resource);
	return result;
}


