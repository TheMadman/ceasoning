#ifndef CSALT_PLATFORM_INIT_H
#define CSALT_PLATFORM_INIT_H

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 700
#endif // #ifndef _XOPEN_SOURCE

#if _XOPEN_SOURCE < 700
#undef _XOPEN_SOURCE
#define _XOPEN_SOURCE 700
#endif // #if _XOPEN_SOURCE < 700

#include <sys/types.h>
#include <limits.h>
#include <fcntl.h>
#include <unistd.h>

#endif // CSALT_PLATFORM_INIT_H
