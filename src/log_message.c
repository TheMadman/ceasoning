#include "csalt/log_message.h"

typedef struct csalt_log_message log_message_t;

static int compare_messages(const void *a, const void *b)
{
	const log_message_t *const first = a;
	const log_message_t *const second = b;
	return first->function != second->function;
}

const char *csalt_log_message_get(
	struct csalt_array array,
	void (*function)(void)
)
{
	log_message_t key = { function, NULL };
	log_message_t *result = csalt_lfind(&key, array, compare_messages);
	if (result)
		return result->message;
	return NULL;
}

