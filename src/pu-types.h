#pragma once

#define PU_STATUSES(CHOOSE_FUNC) \
	CHOOSE_FUNC(PU_SUCCESS, "success") \
	CHOOSE_FUNC(PU_WARNING, "warning") \
	CHOOSE_FUNC(PU_ERROR, "error") \
	CHOOSE_FUNC(PU_ERROR_FATAL, "fatal error")

#define IS_SUCCESS(STATUS) (STATUS == PU_SUCCESS)
#define IS_WARNING(STATUS) (STATUS >= PU_WARNING && STATUS < PU_ERROR)
#define IS_ERROR(STATUS) (STATUS >= PU_ERROR && STATUS < PU_ERROR_FATAL)
#define IS_FATAL(STATUS) (STATUS >= PU_ERROR_FATAL)

#define PU_CHOOSE_ENUM(ENUM, STR) ENUM,
#define PU_CHOOSE_STRING(ENUM, STR) STR,

typedef enum { PU_STATUSES(PU_CHOOSE_ENUM) } pu_status_t;
static const char * PU_STATUS_STRINGS[] = { PU_STATUSES(PU_CHOOSE_STRING) };

typedef struct pu_result_t {
	pu_status_t status;
	void * data;
} pu_result_t;

