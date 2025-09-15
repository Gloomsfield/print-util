#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libusb-1.0/libusb.h>
#include <systemd/sd-device.h>

#include "pu.h"

typedef struct {
	char name[256];
	uint16_t vendor_id;
	uint16_t product_id;
} pu_usb_device;

PU_STATUS_T pu_init_libusb(libusb_context ** libusb_ctx) {
	int libusb_init_result = libusb_init(libusb_ctx);

	if(libusb_init_result) {
		printf("libusb error! 1 %s\n", libusb_error_name(libusb_init_result));
		return PU_FAILURE;
	}

	return PU_SUCCESS;
}

PU_STATUS_T pu_get_usb_devices(
	libusb_context * libusb_ctx,
	uint32_t * out_libusb_device_count,
	libusb_device *** out_libusb_devices,
	sd_device_enumerator ** out_sd_enum,
	uint32_t * out_device_count,
	uint32_t device_buffer_size,
	pu_usb_device * out_device_buffer
) {
	int result = libusb_get_device_list(libusb_ctx, out_libusb_devices);

	if(result < 0) {
		printf("libusb error! %s\n", libusb_error_name(result));
		return PU_FAILURE;
	}

	uint32_t libusb_device_count = result;

	result = sd_device_enumerator_new(out_sd_enum);

	if(result < 0) {
		printf("sd-device error! %s\n", strerror(-result));
		return PU_FAILURE;
	}

	result = sd_device_enumerator_add_match_subsystem(*out_sd_enum, "usb", 1);
	result = sd_device_enumerator_add_match_property(*out_sd_enum, "DEVTYPE", "usb_device");
	
	if(result < 0) {
		printf("sd-device error! %s\n", strerror(-result));
		return PU_FAILURE;
	}
	
	sd_device * device = sd_device_enumerator_get_device_first(*out_sd_enum);

	const char * dev_model_buffer;
	const char * dev_vendor_id_buffer;
	const char * dev_product_id_buffer;

	uint32_t i = 0;
	
	while(device != NULL) {
		if(i >= device_buffer_size) {
			printf("print-util error! device buffer too small!");

			return PU_FAILURE;
		}

		sd_device_get_property_value(device, "ID_MODEL", &dev_model_buffer);
		printf("device: %s\n", dev_model_buffer);

		sd_device_get_property_value(device, "ID_VENDOR_ID", &dev_vendor_id_buffer);
		sd_device_get_property_value(device, "ID_MODEL_ID", &dev_product_id_buffer);

		out_device_buffer[i] = (pu_usb_device){
			.name = "UNKNOWN",
			.vendor_id = strtol(dev_vendor_id_buffer, NULL, 16),
			.product_id = strtol(dev_product_id_buffer, NULL, 16),
		};

		memcpy(out_device_buffer[i].name, dev_model_buffer, strlen(dev_model_buffer + 1));

		printf("vendor: %i, product: %i\n", out_device_buffer[i].vendor_id, out_device_buffer[i].product_id);

		i++;

		device = sd_device_enumerator_get_device_next(*out_sd_enum);
	}

	*out_libusb_device_count = libusb_device_count;

	return PU_SUCCESS;
}

PU_STATUS_T pu_init(pu_context * context) {
	if(pu_init_libusb(&context->libusb_ctx) != PU_SUCCESS) {
		return PU_FAILURE;
	}

	context->initialized = PU_TRUE;
	return PU_SUCCESS;
}

PU_STATUS_T pu_run(pu_context * context) {
	if(context->initialized == PU_FALSE) {
		printf("print-util error! please initialize the context before running it!\n");

		return PU_FAILURE;
	}

	libusb_device ** libusb_devices;
	uint32_t libusb_device_count;

	sd_device_enumerator * sd_enumerator = NULL;

	uint32_t device_count = 0;
	pu_usb_device * device_buffer = malloc(128 * sizeof(pu_usb_device));

	if(pu_get_usb_devices(
		context->libusb_ctx,
		&libusb_device_count,
		&libusb_devices,
		&sd_enumerator,
		&device_count,
		128,
		device_buffer
	) != PU_SUCCESS) {
		printf("print-util error! failed to get usb devices!\n");

		return PU_FAILURE;
	}

	char device_name_buffers[128][256];

	sd_device_enumerator_unref(sd_enumerator);

	libusb_free_device_list(libusb_devices, 1);
	libusb_exit(context->libusb_ctx);

	free(device_buffer);

	return PU_SUCCESS;
}

