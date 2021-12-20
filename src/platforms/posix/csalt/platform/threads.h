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

#ifdef __cplusplus
} // extern "C"
#endif

#endif //CSALT_PLATFORM_THREADS_H 
