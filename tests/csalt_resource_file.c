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

#include "test_macros.h"

#include <csalt/resources/file.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdarg.h>

#define FILENAME "./asdfjkl"

INIT_IMPL(
	int,
	open,
	ARGS(const char *file, int oflag, ...),
	ARGS(file, oflag, 0))

const char *open_file_arg = NULL;
int open_oflag_arg = -1;
int open_return_value = -1;

int open_mock(const char *file, int oflag, ...)
{
	open_file_arg = file;
	open_oflag_arg = oflag;
	return open_return_value;
}

INIT_IMPL(
	int,
	ftruncate,
	ARGS(int fd, off_t length),
	ARGS(fd, length));

int ftruncate_fd_arg = -1;
off_t ftruncate_length_arg = -1;
int ftruncate_return_value = -1;

int ftruncate_mock(int fd, off_t length)
{
	ftruncate_fd_arg = fd;
	ftruncate_length_arg = length;
	return ftruncate_return_value;
}

INIT_IMPL(
	ssize_t,
	pread,
	ARGS(int fd, void *buf, size_t count, off_t offset),
	ARGS(fd, buf, count, offset));

int pread_fd_arg = -1;
void *pread_buf_arg = NULL;
size_t pread_count_arg = 0;
off_t pread_offset_arg = -1;
ssize_t pread_return_value = -1;

ssize_t pread_mock(int fd, void *buf, size_t count, off_t offset)
{
	pread_fd_arg = fd;
	pread_buf_arg = buf;
	pread_count_arg = count;
	pread_offset_arg = offset;
	return pread_return_value;
}

INIT_IMPL(
	ssize_t,
	pwrite,
	ARGS(int fd, const void *buf, size_t count, off_t offset),
	ARGS(fd, buf, count, offset));

int pwrite_fd_arg = -1;
const void *pwrite_buf_arg = NULL;
size_t pwrite_count_arg = 0;
off_t pwrite_offset_arg = -1;
ssize_t pwrite_return_value = -1;

ssize_t pwrite_mock(int fd, const void *buf, size_t count, off_t offset)
{
	pwrite_fd_arg = fd;
	pwrite_buf_arg = buf;
	pwrite_count_arg = count;
	pwrite_offset_arg = offset;
	return pwrite_return_value;
}

INIT_IMPL(
	off_t,
	lseek,
	ARGS(int fd, off_t offset, int whence),
	ARGS(fd, offset, whence));

int lseek_fd_arg = -1;
off_t lseek_offset_arg = -1;
int lseek_whence_arg = -1;
off_t lseek_return_value = -1;

off_t lseek_mock(int fd, off_t offset, int whence)
{
	lseek_fd_arg = fd;
	lseek_offset_arg = offset;
	lseek_whence_arg = whence;
	return lseek_return_value;
}

int test_open(csalt_store *store, void *_)
{
	(void)_;
	if (open_file_arg != FILENAME)
		print_error_and_exit("Filename was unexpected value: %s", open_file_arg);

	// should be opened non-blocking
	if (!(open_oflag_arg & O_NONBLOCK))
		print_error_and_exit("oflag didn't contain O_NONBLOCK flag");

	return 0;
}

int test_read(csalt_store *store, void *_)
{
	(void)_;
	csalt_static_store *s_store = (csalt_static_store *)store;

	char buffer[4096] = { 0 };
	csalt_store_read(s_store, buffer, sizeof(buffer));

	if (pread_buf_arg != &buffer)
		print_error_and_exit("buffer arg was unexpected value: %p", pread_buf_arg);

	if (pread_count_arg != sizeof(buffer))
		print_error_and_exit("count arg was unexpected value: %lu", pread_count_arg);

	if (pread_offset_arg != 0)
		print_error_and_exit("offset arg was unexpected value: %ld", pread_offset_arg);
	return 0;
}

int test_write(csalt_store *store, void *_)
{
	(void)_;
	csalt_static_store *s_store = (csalt_static_store *)store;

	char buffer[] = "Hello, world!";
	ssize_t result = csalt_store_write(s_store, buffer, sizeof(buffer));

	if (result != pwrite_return_value)
		print_error_and_exit("return value was unexpected value: %ld", result);

	if (pwrite_buf_arg != &buffer)
		print_error_and_exit("buffer arg was unexpected value: %p", pwrite_buf_arg);

	if (pwrite_count_arg != sizeof(buffer))
		print_error_and_exit("count arg was unexpected value: %lu", pwrite_count_arg);

	if (pwrite_offset_arg != 0)
		print_error_and_exit("offset arg was unexpected value: %ld", pwrite_offset_arg);

	return 0;
}

int test_resize(csalt_store *store, void *_)
{
	(void)_;

	const ssize_t size = 10;
	ftruncate_return_value = 0;
	ssize_t result = csalt_store_resize(store, size);

	if (result != size)
		print_error_and_exit("Unexpected return value: %ld", result);

	if (ftruncate_length_arg != size)
		print_error_and_exit("Unexpected ftruncate length arg: %ld", ftruncate_length_arg);

	ftruncate_return_value = -1;
	result = csalt_store_resize(store, 0);

	if (result != size)
		print_error_and_exit("Unexpected return value: %ld", result);

	if (ftruncate_length_arg != 0)
		print_error_and_exit("Unexpected ftruncate length arg: %ld", ftruncate_length_arg);
}

int main()
{
	SET_IMPL(open, open_mock);
	SET_IMPL(ftruncate, ftruncate_mock);
	SET_IMPL(pread, pread_mock);
	SET_IMPL(pwrite, pwrite_mock);
	SET_IMPL(lseek, lseek_mock);

	struct csalt_resource_file
		file = csalt_resource_file(FILENAME, O_RDWR, 0644);

	csalt_resource *resource = (csalt_resource *)&file;

	open_return_value = 3;

	if (csalt_resource_use(resource, test_open, NULL) == -1)
		print_error_and_exit("Unexpected error from csalt_resource_use");

	if (csalt_resource_use(resource, test_read, NULL) == -1)
		print_error_and_exit("Unexpected error from csalt_resource_use");

	if (csalt_resource_use(resource, test_write, NULL) == -1)
		print_error_and_exit("Unexpected error from csalt_resource_use");

	if (csalt_resource_use(resource, test_resize, NULL) == -1)
		print_error_and_exit("Unexpected error from csalt_resource_use");

	SET_IMPL(open, dlsym(RTLD_NEXT, "open"));
}
