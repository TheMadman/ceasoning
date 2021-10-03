#include "csalt/stores.h"

#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#include "csalt/util.h"

// virtual call functions

ssize_t csalt_store_read(
	const csalt_store *from,
	void *to,
	size_t bytes
)
{
	return (*from)->read(from, to, bytes);
}

ssize_t csalt_store_write(
	csalt_store *to,
	const void *from,
	size_t bytes
)
{
	return (*to)->write(to, from, bytes);
}

size_t csalt_store_size(const csalt_store *store)
{
	return (*store)->size(store);
}

int csalt_store_split(
	csalt_store *store,
	size_t start,
	size_t end,
	csalt_store_block_fn *block,
	void *data
)
{
	return (*store)->split(store, start, end, block, data);
}

// null interface

ssize_t csalt_store_null_read(
	const csalt_store *from,
	void *to,
	size_t size
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
	size_t size
)
{
	(void)from;
	(void)to;
	(void)size;
	return -1;
}

size_t csalt_store_null_size(const csalt_store *store)
{
	(void)store;
	return 0;
}

int csalt_store_null_split(
	csalt_store *store,
	size_t begin,
	size_t end,
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

// memory interface

ssize_t csalt_memory_read(
	const csalt_store *from,
	void *to,
	size_t size
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
	size_t size
)
{
	struct csalt_memory *memory = (struct csalt_memory *)to;
	if (csalt_store_size(to) < size)
		return -1;

	memcpy(memory->begin, from, size);
	return size;
}

size_t csalt_memory_size(const csalt_store *store)
{
	const struct csalt_memory *memory = (struct csalt_memory *)store;
	return memory->end - memory->begin;
}

int csalt_memory_split(
	csalt_store *store,
	size_t begin,
	size_t end,
	csalt_store_block_fn *block,
	void *data
)
{
	struct csalt_memory *param = (struct csalt_memory *)store;
	struct csalt_memory result = csalt_store_memory_bounds(
		param->begin + begin,
		param->begin + end
	);
	return block((csalt_store *)&result, data);
}

const struct csalt_store_interface csalt_store_memory_implementation = {
	csalt_memory_read,
	csalt_memory_write,
	csalt_memory_size,
	csalt_memory_split,
};

struct csalt_memory csalt_store_memory_bounds(void *begin, void *end)
{
	struct csalt_memory result = {
		&csalt_store_memory_implementation,
		begin,
		end
	};
	return result;
}

void *csalt_store_memory_raw(const struct csalt_memory *memory)
{
	return memory->begin;
}

// Transfer algorithmm

struct csalt_progress csalt_progress(size_t size)
{
	struct csalt_progress result = {
		size,
		0
	};
	return result;
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
	size_t size = data->progress->total - data->progress->amount_completed;
	if (size == 0)
		return 0;

	size_t transfer_size = min(size, DEFAULT_PAGESIZE);

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
	csalt_store *to = data->to;

	return csalt_store_split(
		from,
		data->progress->amount_completed,
		data->progress->total,
		receive_split_from,
		data
	);
}

static size_t progress_remaining(struct csalt_progress *progress)
{
	return progress->total - progress->amount_completed;
}

ssize_t csalt_store_transfer(
	struct csalt_progress *progress,
	csalt_store *to,
	csalt_store *from,
	csalt_transfer_complete_fn *callback
)
{
	char buffer[DEFAULT_PAGESIZE] = { 0 };
	struct transfer_data data = { to, from, buffer, progress };

	size_t remaining;
	while ((remaining = progress_remaining(progress))) {
		ssize_t this_write = csalt_store_transfer_real(&data);
		if (this_write < 0) {
			progress->amount_completed = this_write;
			break;
		}

		progress->amount_completed += this_write;
		if (this_write < sizeof(buffer))
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
	csalt_fd result = {
		&file_descriptor_interface,
		fd,
		max(seek_end, 0),
	};

	lseek(fd, 0, SEEK_SET);

	return result;
}

ssize_t csalt_store_file_descriptor_read(const csalt_store *store, void *buffer, size_t bytes)
{
	const csalt_fd *file = (csalt_fd *)store;
	return read(file->fd, buffer, bytes);
}

ssize_t csalt_store_file_descriptor_write(csalt_store *store, const void *buffer, size_t bytes)
{
	csalt_fd *file = (csalt_fd *)store;
	return write(file->fd, buffer, bytes);
}

size_t csalt_store_file_descriptor_size(const csalt_store *store)
{
	// probably racy
	const csalt_fd *file = (const csalt_fd *)store;
	off_t current = max(lseek(file->fd, 0, SEEK_CUR), 0);

	// in the case that the file was "split", we only want
	// to report the size from the current split beginning
	// (I.E. the current seek offset) to the current split end
	off_t result = file->split_end - current;
	lseek(file->fd, current, SEEK_SET);

	return result;
}

int csalt_store_file_descriptor_split(
	csalt_store *store,
	size_t begin,
	size_t end,
	csalt_store_block_fn *block,
	void *param
)
{
	csalt_fd *fd = (csalt_fd *)store;
	off_t current = max(lseek(fd->fd, 0, SEEK_CUR), 0);
	max(lseek(fd->fd, current + begin, SEEK_SET), 0);

	// Any reason to create a new csalt_fd on the stack?
	// Can't think of one
	int result = block(store, param);

	lseek(fd->fd, current, SEEK_SET);
	return result;
}

