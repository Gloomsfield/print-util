#pragma once

#include <stdio.h>

#define STATUSES(CHOOSE_FUNC) \
	CHOOSE_FUNC(FAILURE, "generic failure!", -1) \
	CHOOSE_FUNC(SUCCESS, "success!", 0)

#define CHOOSE_ENUM(ENUM, STRING, EXIT_CODE) ENUM,
#define CHOOSE_STRING(ENUM, STRING, EXIT_CODE) STRING,
#define CHOOSE_EXIT_CODE(ENUM, STRING, EXIT_CODE) EXIT_CODE,

typedef enum status_t { STATUSES(CHOOSE_ENUM) } status_t;
static char* status_strings[] = { STATUSES(CHOOSE_STRING) };
static int status_exit_codes[] = { STATUSES(CHOOSE_EXIT_CODE) };

#define ASSERT(condition, cleanup, state, fail_status) { \
	if(!(condition)) { \
		cleanup((state)); \
		fprintf(stderr, "print-util error: %s\n", status_strings[(fail_status)]); \
	} \
}
