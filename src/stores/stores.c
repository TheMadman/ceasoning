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

#include "csalt/stores.h"

#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "csalt/util.h"

// virtual call functions

ssize_t csalt_store_read(
	csalt_store *from,
	void *to,
	ssize_t bytes
)
{
	return (*from)->read(from, to, bytes);
}

ssize_t csalt_store_write(
	csalt_store *to,
	const void *from,
	ssize_t bytes
)
{
	return (*to)->write(to, from, bytes);
}

ssize_t csalt_store_size(csalt_store *store)
{
	return (*store)->size(store);
}

int csalt_store_split(
	csalt_store *store,
	ssize_t start,
	ssize_t end,
	csalt_store_block_fn *block,
	void *data
)
{
	return (*store)->split(store, start, end, block, data);
}

// null interface

ssize_t csalt_store_null_read(
	csalt_store *from,
	void *to,
	ssize_t size
)
{
	/*
	 * We create (void) statements to prevent compiler warnings
	 * or errors on unused parameters.
	 *
	 * Those warnings are good to have so writing explicitely that
	 * we want to ignore them will help our compiler help us
	 * more.
	 */
	(void)from;
	(void)to;
	(void)size;
	return -1;
}

ssize_t csalt_store_null_write(
	csalt_store *from,
	const void *to,
	ssize_t size
)
{
	(void)from;
	(void)to;
	(void)size;
	return -1;
}

ssize_t csalt_store_null_size(csalt_store *store)
{
	(void)store;
	return 0;
}

int csalt_store_null_split(
	csalt_store *store,
	ssize_t begin,
	ssize_t end,
	csalt_store_block_fn *block,
	void *data
)
{
	(void)store;
	(void)begin;
	(void)end;
	(void)block;
	(void)data;

	return 0;
}

const struct csalt_store_interface csalt_store_null_interface = {
	csalt_store_null_read,
	csalt_store_null_write,
	csalt_store_null_size,
	csalt_store_null_split,
};

const struct csalt_store_interface *csalt_store_null_implementation = &csalt_store_null_interface;
csalt_store *csalt_store_null = &csalt_store_null_implementation;

// memory interface

ssize_t csalt_memory_read(
	csalt_store *from,
	void *to,
	ssize_t size
)
{
	struct csalt_memory *memory = (struct csalt_memory *)from;
	if (memory->end - memory->begin < size)
		return -1;

	memcpy(to, memory->begin, size);
	return size;
}

ssize_t csalt_memory_write(
	csalt_store *to,
	const void *from,
	ssize_t size
)
{
	struct csalt_memory *memory = (struct csalt_memory *)to;
	if (csalt_store_size(to) < size)
		return -1;

	memcpy(memory->begin, from, size);
	return size;
}

ssize_t csalt_memory_size(csalt_store *store)
{
	const struct csalt_memory *memory = (struct csalt_memory *)store;
	return memory->end - memory->begin;
}

int csalt_memory_split(
	csalt_store *store,
	ssize_t begin,
	ssize_t end,
	csalt_store_block_fn *block,
	void *data
)
{
	struct csalt_memory *param = (struct csalt_memory *)store;
	struct csalt_memory result = csalt_memory_bounds(
		max(param->begin, min(param->begin + begin, param->end)),
		max(param->begin, min(param->begin + end, param->end))
	);
	return block((csalt_store *)&result, data);
}

const struct csalt_store_interface csalt_memory_implementation = {
	csalt_memory_read,
	csalt_memory_write,
	csalt_memory_size,
	csalt_memory_split,
};

struct csalt_memory csalt_memory_bounds(void *begin, void *end)
{
	struct csalt_memory result = {
		&csalt_memory_implementation,
		begin,
		end
	};
	return result;
}

void *csalt_memory_raw(const struct csalt_memory *memory)
{
	return memory->begin;
}

ssize_t csalt_cmemory_read(
	csalt_store *from,
	void *to,
	ssize_t size
)
{
	struct csalt_cmemory
		*memory = (struct csalt_cmemory *)from;

	if (memory->end - memory->begin < size)
		return -1;

	memcpy(to, memory->begin, size);
	return size;
}

ssize_t csalt_cmemory_size(csalt_store *store)
{
	struct csalt_cmemory
		*memory = (struct csalt_cmemory *)store;
	return memory->end - memory->begin;
}

int csalt_cmemory_split(
	csalt_store *store,
	ssize_t begin,
	ssize_t end,
	csalt_store_block_fn *block,
	void *data
)
{
	struct csalt_cmemory
		*memory = (struct csalt_cmemory *)store;
	struct csalt_cmemory result = csalt_cmemory_bounds(
		max(memory->begin, min(memory->begin + begin, memory->end)),
		max(memory->begin, min(memory->begin + end, memory->end))
	);
	return block((csalt_store *)&result, data);
}

const struct csalt_store_interface csalt_cmemory_implementation = {
	csalt_cmemory_read,
	csalt_store_null_write,
	csalt_cmemory_size,
	csalt_cmemory_split,
};

struct csalt_cmemory csalt_cmemory_bounds(const void *begin, const void *end)
{
	return (struct csalt_cmemory) {
		&csalt_cmemory_implementation,
		begin,
		end,
	};
}

// Transfer algorithmm

struct csalt_progress csalt_progress(ssize_t size)
{
	struct csalt_progress result = {
		size,
		0
	};
	return result;
}

int csalt_progress_complete(struct csalt_progress *progress)
{
	return progress->total == progress->amount_completed;
}

struct transfer_data {
	csalt_store *to;
	csalt_store *from;
	char *buffer;
	struct csalt_progress *progress;
};

static ssize_t csalt_store_transfer_real(struct transfer_data *);

static int receive_split_to(csalt_store *to, void *data_pointer)
{
	struct transfer_data *data = data_pointer;
	char *buffer = data->buffer;
	csalt_store *from = data->from;
	ssize_t size = data->progress->total - data->progress->amount_completed;
	if (size == 0)
		return 0;

	ssize_t transfer_size = min(size, DEFAULT_PAGESIZE);

	ssize_t read_size = csalt_store_read(from, buffer, transfer_size);
	if (read_size < 0)
		return -1;
	ssize_t write_size = csalt_store_write(to, buffer, read_size);

	return write_size;
}

static int receive_split_from(csalt_store *from, void *data_pointer)
{
	struct transfer_data *data = data_pointer;
	struct transfer_data new_data = *data;
	new_data.from = from;

	return csalt_store_split(
		data->to,
		data->progress->amount_completed,
		data->progress->total,
		receive_split_to,
		&new_data
	);
}

static ssize_t csalt_store_transfer_real(struct transfer_data *data)
{
	csalt_store *from = data->from;

	return csalt_store_split(
		from,
		data->progress->amount_completed,
		data->progress->total,
		receive_split_from,
		data
	);
}

static ssize_t progress_remaining(struct csalt_progress *progress)
{
	return progress->total - progress->amount_completed;
}

ssize_t csalt_store_transfer(
	struct csalt_progress *progress,
	csalt_store *from,
	csalt_store *to,
	csalt_transfer_complete_fn *callback
)
{
	char buffer[DEFAULT_PAGESIZE] = { 0 };
	struct transfer_data data = { to, from, buffer, progress };

	ssize_t remaining;
	while ((remaining = progress_remaining(progress))) {
		ssize_t this_write = csalt_store_transfer_real(&data);
		if (this_write < 0) {
			progress->amount_completed = this_write;
			break;
		}

		progress->amount_completed += this_write;
		if (this_write < (ssize_t)sizeof(buffer))
			break;
	}

	if (callback && !progress_remaining(progress)) {
		callback(to);
	}

	return progress->amount_completed;
}


/*
 * For most error cases when seeking the file, we consider
 * the operation to be successful - most error cases are
 * seeking before the beginning/after the end of a device
 * (which we don't do), or the file descriptor isn't a seekable
 * device (such as stdin, stdout, stderr).
 *
 * A bad file descriptor is revealed by attempting to read/write
 * from the device as well, so programmers shouldn't struggle
 * tracking that error down, even if we hide it when seeking.
 *
 * I'm not sure I like this policy, but returning an error on
 * construction/split/size. is something I've not considered,
 * since most constructors are just storing the arguments for use
 * in read/write calls.
 */
struct csalt_store_interface file_descriptor_interface = {
	csalt_store_file_descriptor_read,
	csalt_store_file_descriptor_write,
	csalt_store_file_descriptor_size,
	csalt_store_file_descriptor_split,
};

typedef struct csalt_store_file_descriptor csalt_fd;

csalt_fd csalt_store_file_descriptor(int fd)
{
	off_t seek_end = lseek(fd, 0, SEEK_END);
	off_t seek_begin = lseek(fd, 0, SEEK_SET);
	csalt_fd result = {
		&file_descriptor_interface,
		fd,
		max(seek_begin, 0),
		max(seek_end, 0),
	};

	return result;
}

ssize_t csalt_store_file_descriptor_read(csalt_store *store, void *buffer, ssize_t bytes)
{
	const csalt_fd *file = (csalt_fd *)store;
	ssize_t result = read(file->fd, buffer, bytes);
	if (result < 0 && (errno & (EWOULDBLOCK | EAGAIN))) {
		result = 0;
	}
	return result;
}

ssize_t csalt_store_file_descriptor_write(csalt_store *store, const void *buffer, ssize_t bytes)
{
	csalt_fd *file = (csalt_fd *)store;
	ssize_t result = write(file->fd, buffer, bytes);
	if (result < 0 && (errno & (EWOULDBLOCK | EAGAIN))) {
		result = 0;
	} else if (file->begin + result > file->end) {
		file->end = max(lseek(file->fd, 0, SEEK_END), 0);
	}
	return result;
}

ssize_t csalt_store_file_descriptor_size(csalt_store *store)
{
	const csalt_fd *file = (const csalt_fd *)store;
	return file->end - file->begin;
}

int csalt_store_file_descriptor_split(
	csalt_store *store,
	ssize_t begin,
	ssize_t end,
	csalt_store_block_fn *block,
	void *param
)
{
	csalt_fd *fd = (csalt_fd *)store;
	off_t current = max(lseek(fd->fd, 0, SEEK_CUR), 0);
	ssize_t new_begin = current + begin;
	ssize_t new_end = min(new_begin + end, fd->end);
	max(lseek(fd->fd, new_begin, SEEK_SET), 0);

	csalt_fd new_fd = { fd->vtable, fd->fd, new_begin, new_end };
	csalt_store *new_store = (csalt_store *)&new_fd;

	int result = block(new_store, param);

	if (new_fd.end > fd->end) {
		// data written past the previous end of the file,
		// update current file
		fd->end = max(lseek(fd->fd, 0, SEEK_END), 0);
	}

	lseek(fd->fd, current, SEEK_SET);
	return result;
}

