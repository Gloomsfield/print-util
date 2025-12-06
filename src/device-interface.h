#pragma once

#include <libusb-1.0/libusb.h>

#include <inttypes.h>

#include "statuses.h"

#define PU_DEVICE_BUFFER_SIZE (256)

typedef struct pu_usb_device_t {
	char name[256];
	uint16_t vendor_id;
	uint16_t product_id;
} pu_usb_device_t;

typedef struct pu_device_buffer_t {
	pu_usb_device_t buffer[PU_DEVICE_BUFFER_SIZE];
	uint32_t size;
} pu_device_buffer_t;

typedef struct libusb_device_context_t {
	libusb_context * context;

	libusb_device ** devices;
	uint32_t device_count;
} libusb_device_context_t;

typedef struct libusb_device_t {
	libusb_device_handle * handle;

	uint8_t interface_number;
	uint8_t endpoint_number;
} libusb_device_t;

