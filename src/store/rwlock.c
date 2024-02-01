#include "csalt/store/rwlock.h"

typedef struct csalt_store_rwlock rwlock_t;

static const struct csalt_dynamic_store_interface impl = {
	{
		csalt_store_rwlock_read,
		csalt_store_rwlock_write,
		csalt_store_rwlock_split,
	},
	csalt_store_rwlock_size,
	csalt_store_rwlock_resize,
};

struct csalt_store_rwlock csalt_store_rwlock(
	csalt_store *store,
	csalt_rwlock *lock
)
{
	return (rwlock_t) {
		{
			.vtable = &impl,
			.decorated = store,
		},
		.lock = lock,
	};
}

ssize_t csalt_store_rwlock_read(
	csalt_static_store *store,
	void *buffer,
	ssize_t amount
)
{
	rwlock_t *const lock = (rwlock_t *)store;
	if (csalt_rwlock_tryrdlock(lock->lock))
		return -1;
	const ssize_t result = csalt_store_read(
		lock->parent.decorated_static,
		buffer,
		amount);
	csalt_rwlock_unlock(lock->lock);
	return result;
}

ssize_t csalt_store_rwlock_write(
	csalt_static_store *store,
	const void *buffer,
	ssize_t amount
)
{
	rwlock_t *const lock = (rwlock_t *)store;
	if (csalt_rwlock_trywrlock(lock->lock))
		return -1;
	const ssize_t result = csalt_store_write(
		lock->parent.decorated_static,
		buffer,
		amount);
	csalt_rwlock_unlock(lock->lock);
	return result;
}

struct split {
	rwlock_t *lock;
	csalt_static_store_block_fn *block;
	void *param;
};

static int receive_split(csalt_static_store *store, void *param)
{
	struct split *params = param;
	rwlock_t new_lock = csalt_store_rwlock(
		(csalt_store *)store,
		params->lock->lock);
	return params->block(
		(csalt_static_store *)&new_lock,
		params->param);
}

int csalt_store_rwlock_split(
	csalt_static_store *store,
	ssize_t begin,
	ssize_t end,
	csalt_static_store_block_fn *block,
	void *param
)
{
	rwlock_t *lock = (rwlock_t *)store;
	struct split split = {
		lock,
		block,
		param,
	};

	const int result = csalt_store_split(
		lock->parent.decorated_static,
		begin,
		end,
		receive_split,
		&split);

	return result;
}

ssize_t csalt_store_rwlock_size(csalt_store *store)
{
	rwlock_t *lock = (rwlock_t *)store;
	if (csalt_rwlock_tryrdlock(lock->lock))
		return -1;
	const ssize_t result = csalt_store_size(lock->parent.decorated);
	csalt_rwlock_unlock(lock->lock);
	return result;
}

ssize_t csalt_store_rwlock_resize(
	csalt_store *store,
	ssize_t new_size
)
{
	rwlock_t *lock = (rwlock_t *)store;
	if (csalt_rwlock_trywrlock(lock->lock))
		return -1;
	const ssize_t result = csalt_store_resize(
		lock->parent.decorated,
		new_size);
	csalt_rwlock_unlock(lock->lock);
	return result;
}
