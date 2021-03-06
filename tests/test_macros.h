#ifndef TEST_MACROS_H
#define TEST_MACROS_H

#include <stdio.h>
#define EXIT_SUCCESS 0
#define EXIT_TEST_FAILURE 99
#define EXIT_TEST_SKIPPED 77
#define print_error(...) do {	\
	fprintf(stderr, "%s:%d: ", __FILE__, __LINE__);	\
	fprintf(stderr, __VA_ARGS__);	\
	fprintf(stderr, "\n");	\
} while (0)

#endif // TEST_MACROS_H
