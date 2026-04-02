#pragma once

#define PU_STATUSES(CHOOSE) \
	CHOOSE(PU_SUCCESS, "print-util generic success") \
	CHOOSE(PU_WARNING, "print-util generic warning") \
	CHOOSE(PU_ERROR, "print-util generic error")

#define PU_CHOOSE_ENUM(ENUM, STRING) ENUM,
#define PU_CHOOSE_STRING(ENUM, STRING) STRING,

typedef enum { PU_STATUSES(PU_CHOOSE_ENUM) } pu_status_t;
static const char * pu_status_strings[] = { PU_STATUSES(PU_CHOOSE_STRING) };

#define pu_trace(STATUS, FUNCTION_NAME) { printf("%s: %s", FUNCTION_NAME, pu_status_strings[STATUS]); }

#define pu_propagate(STATUS, THRESHOLD) { \
	if(STATUS >= THRESHOLD) { \
		pu_trace(STATUS, __func__); \
		return STATUS; \
	} \
}

