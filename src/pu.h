#pragma once

#include <libusb-1.0/libusb.h>

#include "pu-constants.h"

#define PU_STATUSES(CHOOSE_FUNC) \
	CHOOSE_FUNC(PU_SUCCESS, "print-util success") \
	CHOOSE_FUNC(PU_FAILURE, "print-util failure")

#define PU_CHOOSE_ENUM(ENUM, STR) ENUM,
#define PU_CHOOSE_STRING(ENUM, STR) STR,

typedef enum { PU_STATUSES(PU_CHOOSE_ENUM) } PU_STATUS_T;
static const char * PU_STATUS_STRINGS[] = { PU_STATUSES(PU_CHOOSE_STRING) };

typedef enum { PU_TRUE, PU_FALSE } PU_BOOL_T;

typedef struct {
	char name[256];
	uint16_t vendor_id;
	uint16_t product_id;
} pu_usb_device;

typedef struct {
	PU_BOOL_T initialized;
	libusb_context * libusb_ctx;
	libusb_device ** libusb_devices;
	uint32_t libusb_device_count;
	libusb_device_handle * libusb_printer_handle;
	pu_usb_device device_buffer[PU_DEVICE_BUFFER_SIZE];
	uint32_t device_count;
} pu_context;

PU_STATUS_T pu_init(pu_context * context);
PU_STATUS_T pu_run(pu_context * context);

