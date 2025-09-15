#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libusb-1.0/libusb.h>
#include <systemd/sd-device.h>

#include "pu.h"
#include "pu-constants.h"

PU_STATUS_T pu_init_libusb(libusb_context ** libusb_ctx) {
	int libusb_init_result = libusb_init(libusb_ctx);

	if(libusb_init_result) {
		printf("libusb error! 1 %s\n", libusb_error_name(libusb_init_result));
		return PU_FAILURE;
	}

	return PU_SUCCESS;
}

PU_STATUS_T pu_get_usb_devices(pu_context * context) {
	int result = libusb_get_device_list(context->libusb_ctx, &context->libusb_devices);

	if(result < 0) {
		printf("libusb error! %s\n", libusb_error_name(result));
		return PU_FAILURE;
	}

	uint32_t libusb_device_count = result;

	sd_device_enumerator * sd_enum;

	result = sd_device_enumerator_new(&sd_enum);

	if(result < 0) {
		printf("sd-device error! %s\n", strerror(-result));
		return PU_FAILURE;
	}

	result = sd_device_enumerator_add_match_subsystem(sd_enum, "usb", 1);
	result = sd_device_enumerator_add_match_property(sd_enum, "DEVTYPE", "usb_device");
	
	if(result < 0) {
		printf("sd-device error! %s\n", strerror(-result));
		return PU_FAILURE;
	}
	
	sd_device * device = sd_device_enumerator_get_device_first(sd_enum);

	const char * dev_model_buffer;
	const char * dev_vendor_id_buffer;
	const char * dev_product_id_buffer;

	uint32_t i = 0;
	
	while(device != NULL) {
		if(i >= PU_DEVICE_BUFFER_SIZE) {
			printf("print-util error! device buffer too small!");

			return PU_FAILURE;
		}

		sd_device_get_property_value(device, "ID_MODEL", &dev_model_buffer);
		printf("device: %s\n", dev_model_buffer);

		sd_device_get_property_value(device, "ID_VENDOR_ID", &dev_vendor_id_buffer);
		sd_device_get_property_value(device, "ID_MODEL_ID", &dev_product_id_buffer);

		context->device_buffer[i] = (pu_usb_device){
			.name = "UNKNOWN",
			.vendor_id = strtol(dev_vendor_id_buffer, NULL, 16),
			.product_id = strtol(dev_product_id_buffer, NULL, 16),
		};

		memcpy(context->device_buffer[i].name, dev_model_buffer, strlen(dev_model_buffer + 1));

		printf("vendor: %i, product: %i\n", context->device_buffer[i].vendor_id, context->device_buffer[i].product_id);

		i++;

		device = sd_device_enumerator_get_device_next(sd_enum);
	}

	context->libusb_device_count = libusb_device_count;

	sd_device_enumerator_unref(sd_enum);

	return PU_SUCCESS;
}

PU_STATUS_T pu_init(pu_context * context) {
	if(pu_init_libusb(&context->libusb_ctx) != PU_SUCCESS) {
		return PU_FAILURE;
	}

	context->initialized = PU_TRUE;
	return PU_SUCCESS;
}

void pu_cleanup(pu_context * context) {
	libusb_free_device_list(context->libusb_devices, 1);
	libusb_exit(context->libusb_ctx);
}

PU_STATUS_T pu_run(pu_context * context) {
	if(context->initialized == PU_FALSE) {
		printf("print-util error! please initialize the context before running it!\n");

		return PU_FAILURE;
	}

	if(pu_get_usb_devices(context) != PU_SUCCESS) {
		printf("print-util error! failed to get usb devices!\n");

		pu_cleanup(context);

		return PU_FAILURE;
	}

	pu_cleanup(context);

	return PU_SUCCESS;
}

