#ifndef CSALT_PLATFORM_THREADS_H
#define CSALT_PLATFORM_THREADS_H

#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef pthread_mutex_t csalt_mutex;

// should standardize this
#define csalt_mutex_init(...) pthread_mutex_init(__VA_ARGS__)
#define csalt_mutex_lock(mutex) pthread_mutex_lock(mutex)
#define csalt_mutex_trylock(mutex) pthread_mutex_trylock(mutex)
#define csalt_mutex_unlock(mutex) pthread_mutex_unlock(mutex)
#define csalt_mutex_deinit(mutex) pthread_mutex_destroy(mutex)

typedef pthread_rwlock_t csalt_rwlock;

#define csalt_rwlock_init(...) pthread_rwlock_init(__VA_ARGS__)
#define csalt_rwlock_rdlock(rwlock) pthread_rwlock_rdlock(rwlock)
#define csalt_rwlock_wrlock(rwlock) pthread_rwlock_wrlock(rwlock)
#define csalt_rwlock_tryrdlock(rwlock) pthread_rwlock_tryrdlock(rwlock)
#define csalt_rwlock_trywrlock(rwlock) pthread_rwlock_trywrlock(rwlock)
#define csalt_rwlock_unlock(rwlock) pthread_rwlock_unlock(rwlock)
#define csalt_rwlock_deinit(rwlock) pthread_rwlock_destroy(rwlock)

#ifdef __cplusplus
} // extern "C"
#endif

#endif //CSALT_PLATFORM_THREADS_H 
