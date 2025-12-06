#pragma once

#include <libusb-1.0/libusb.h>

#include "pu-constants.h"

#define PU_STATUSES(CHOOSE_FUNC) \
	CHOOSE_FUNC(PU_SUCCESS, "print-util success") \
	CHOOSE_FUNC(PU_FAILURE, "print-util failure")

#define PU_CHOOSE_ENUMS(ENUM, STR) ENUM,
#define PU_CHOOSE_STRINGS(ENUM, STR) STR,

typedef enum { PU_STATUSES(PU_CHOOSE_ENUMS) } PU_STATUS_T;
static const char * PU_STATUS_STRINGS[] = { PU_STATUSES(PU_CHOOSE_STRINGS) };

typedef enum { PU_TRUE, PU_FALSE } PU_BOOL_T;

typedef struct {
	char name[256];
	uint16_t vendor_id;
	uint16_t product_id;
} pu_usb_device;

typedef struct {
	PU_BOOL_T initialized;

	PU_BOOL_T prompt_for_printer;

	char printer_identifier[256];
	
	char print_text[256];
	char print_heading[256];

	libusb_context * libusb_ctx;
	libusb_device ** libusb_devices;
	uint32_t libusb_device_count;
	libusb_device_handle * libusb_printer_handle;
	uint8_t libusb_device_interface_number;
	uint8_t libusb_device_endpoint_address;
	pu_usb_device device_buffer[PU_DEVICE_BUFFER_SIZE];
	uint32_t device_count;
} pu_context_t;

typedef struct {
	uint8_t data[PU_MAX_IMAGE_WIDTH * PU_MAX_IMAGE_HEIGHT / 8];
	uint16_t height;
	uint16_t width;
} pu_image_t;

static pu_context_t global_context = {
	.initialized = PU_FALSE,
	.prompt_for_printer = PU_FALSE,
	.printer_identifier = { 0 },
};

PU_STATUS_T pu_init(int argc, char * argv[]);
PU_STATUS_T pu_run();

