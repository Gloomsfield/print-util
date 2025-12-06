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

typedef struct pu_usb_device_t {
	char name[256];
	uint16_t vendor_id;
	uint16_t product_id;
} pu_usb_device_t;

typedef struct pu_context_t {
	uint8_t initialized;

	char * config_file_path;

	uint8_t set_default_printer;


	libusb_context * libusb_ctx;
	libusb_device ** libusb_devices;
	uint32_t libusb_device_count;
	libusb_device_handle * libusb_printer_handle;
	uint8_t libusb_device_interface_number;
	uint8_t libusb_device_endpoint_address;
	pu_usb_device_t device_buffer[PU_DEVICE_BUFFER_SIZE];
	uint32_t device_count;

	uint8_t kernel_was_active;
} pu_context_t;

typedef enum pu_cutmode_t {
	PU_CUTMODE_NONE,
	PU_CUTMODE_FULL,
	PU_CUTMODE_PART,
} pu_cutmode_t;

typedef enum pu_justification_t {
	PU_JUSTIFICATION_LEFT,
	PU_JUSTIFICATION_CENTER,
	PU_JUSTIFICATION_RIGHT,
} pu_justification_t;

typedef struct pu_text_t {
	uint8_t emphasized;
	pu_justification_t justification;
	uint8_t font_index;

	uint8_t character_width;
	uint8_t character_height;
	
	char * string;

	pu_cutmode_t cutmode;
} pu_text_t;

typedef struct pu_image_t {
	uint8_t data[PU_MAX_IMAGE_WIDTH * PU_MAX_IMAGE_HEIGHT / 8];

	uint16_t height;
	uint16_t width;

	pu_cutmode_t cutmode;
} pu_image_t;

PU_STATUS_T pu_init(int argc, char * argv[]);
PU_STATUS_T pu_select();
PU_STATUS_T pu_print_text(pu_text_t * text);
PU_STATUS_T pu_print_image(pu_image_t * image);
PU_STATUS_T pu_stop();

