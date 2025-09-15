#pragma once

#include <libusb-1.0/libusb.h>

#define PU_STATUSES(CHOOSE_FUNC) \
	CHOOSE_FUNC(PU_SUCCESS, "print-util success") \
	CHOOSE_FUNC(PU_FAILURE, "print-util failure")

#define PU_CHOOSE_ENUM(ENUM, STR) ENUM,
#define PU_CHOOSE_STRING(ENUM, STR) STR,

typedef enum { PU_STATUSES(PU_CHOOSE_ENUM) } PU_STATUS_T;
static const char * PU_STATUS_STRINGS[] = { PU_STATUSES(PU_CHOOSE_STRING) };

typedef enum { PU_TRUE, PU_FALSE } PU_BOOL;

typedef struct {
	PU_BOOL initialized;
	libusb_context * libusb_ctx;
} pu_context;

PU_STATUS_T pu_init(pu_context * context);
PU_STATUS_T pu_run(pu_context * context);
