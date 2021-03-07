#include <csalt/fileresource.h>

#include <unistd.h>
#include <fcntl.h>
#include <csalt/util.h>

struct csalt_resource_interface csalt_resource_file_interface = {
	{
		csalt_resource_file_read,
		csalt_resource_file_write,
		csalt_resource_file_size,
		csalt_resource_file_split,
	},
	csalt_resource_file_init,
	csalt_resource_file_valid,
	csalt_resource_file_deinit,
};

ssize_t csalt_resource_file_read(const csalt_store *store, void *buffer, size_t size)
{
	struct csalt_resource_file *file = castto(file, store);
	lseek(file->fd, file->begin, SEEK_SET);
	return read(file->fd, buffer, size);
}

ssize_t csalt_resource_file_write(csalt_store *store, const void *buffer, size_t size)
{
	struct csalt_resource_file *file = castto(file, store);
	lseek(file->fd, file->begin, SEEK_SET);
	return write(file->fd, buffer, size);
}

size_t csalt_resource_file_size(const csalt_store *store)
{
	struct csalt_resource_file *file = castto(file, store);
	return file->end - file->begin;
}

int csalt_resource_file_split(
	csalt_store *store,
	size_t begin,
	size_t end,
	csalt_store_block_fn *block,
	void *data
)
{
	struct csalt_resource_file *file = castto(file, store);
	struct csalt_resource_file result = *file;
	result.end = result.begin + end;
	result.begin += begin;
	return block(castto(csalt_store *, &result), data);
}

void csalt_resource_file_init(csalt_resource *resource)
{
	struct csalt_resource_file *file = castto(file, resource);

	// Some open flags break the API for reading/writing
	int banned_flags = O_APPEND;
	file->fd = open(file->filename, file->flags & ~banned_flags);
	if (csalt_resource_file_valid(resource)) {
		file->end = lseek(file->fd, 0, SEEK_END);
		file->begin = lseek(file->fd, 0, SEEK_SET);
	}
}

char csalt_resource_file_valid(const csalt_resource *resource)
{
	struct csalt_resource_file *file = castto(file, resource);
	return file->fd >= 0;
}

void csalt_resource_file_deinit(csalt_resource *resource)
{
	struct csalt_resource_file *file = castto(file, resource);
	close(file->fd);
	file->fd = -1;
}

struct csalt_resource_file cslt_resource_file(const char *path, int flags)
{
	struct csalt_resource_file file = {
		&csalt_resource_file_interface,
		path,
		flags,
		-1,
		0
	};
	return file;
}

