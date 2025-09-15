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

		sd_device_get_property_value(device, "ID_VENDOR_ID", &dev_vendor_id_buffer);
		sd_device_get_property_value(device, "ID_MODEL_ID", &dev_product_id_buffer);

		context->device_buffer[i] = (pu_usb_device){
			.name = "UNKNOWN",
			.vendor_id = strtol(dev_vendor_id_buffer, NULL, 16),
			.product_id = strtol(dev_product_id_buffer, NULL, 16),
		};

		memcpy(context->device_buffer[i].name, dev_model_buffer, strlen(dev_model_buffer));

		i++;

		device = sd_device_enumerator_get_device_next(sd_enum);
	}

	context->device_count = i;

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

PU_STATUS_T pu_choose_device(pu_context * context, uint32_t device_index) {
	context->libusb_printer_handle = libusb_open_device_with_vid_pid(
		context->libusb_ctx,
		context->device_buffer[device_index].vendor_id,
		context->device_buffer[device_index].product_id
	);

	if(context->libusb_printer_handle == NULL) {
		return PU_FAILURE;
	}

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
		printf("print-util error! failed to get USB devices!\n");

		pu_cleanup(context);

		return PU_FAILURE;
	}

	if(context->device_count == 0) {
		printf("print-util error! no USB devices found!\n");

		return PU_FAILURE;
	}

	printf("please select your printer from the following list of USB devices:\n");

	for(uint32_t i = 0; i < context->device_count; i++) {
		printf("%u - %s\n", i + 1, context->device_buffer[i].name);
	}

	uint32_t device_choice = 0;

	while(device_choice < 1 || context->device_count < device_choice) {
		printf("[1, %u]: ", context->device_count);

		scanf("%d", &device_choice);
	}

	if(pu_choose_device(context, device_choice - 1) != PU_SUCCESS) {
		printf("print-util error! failed to choose device!\n");
		return PU_FAILURE;
	}

	unsigned char data[] = { 0x1d, 0x21, 0x11, 'w', 'h', 'o', 'a', 0x0a, 0x1d, 'V', 66, 0 };
	int bytes_transferred = 0;

	struct libusb_config_descriptor * config_descriptor;

	int result = 0;

	result = libusb_get_active_config_descriptor(libusb_get_device(context->libusb_printer_handle), &config_descriptor);

	if(result < 0) {
		printf("libusb error! failed to get active config descriptor: %s\n", libusb_error_name(result));

		pu_cleanup(context);

		return PU_FAILURE;
	}

	uint8_t interface_number = 0;
	uint8_t endpoint_address = 0;

	PU_BOOL_T found_endpoint = PU_FALSE;

	for(uint32_t i = 0; i < config_descriptor->bNumInterfaces && found_endpoint == PU_FALSE; i++) {
		const struct libusb_interface * interface = &config_descriptor->interface[i];

		for(uint32_t j = 0; j < interface->num_altsetting && found_endpoint == PU_FALSE; j++) {
			const struct libusb_interface_descriptor * altsetting = &interface->altsetting[j];

			for(uint32_t k = 0; k < altsetting->bNumEndpoints; k++) {
				const struct libusb_endpoint_descriptor * endpoint = &altsetting->endpoint[k];

				if((endpoint->bmAttributes & LIBUSB_TRANSFER_TYPE_MASK) != LIBUSB_TRANSFER_TYPE_BULK) {
					continue;
				}

				uint8_t temp_endpoint_address = endpoint->bEndpointAddress;

				if((temp_endpoint_address & LIBUSB_ENDPOINT_DIR_MASK) == LIBUSB_ENDPOINT_IN) {
					continue;
				}

				interface_number = i;
				endpoint_address = temp_endpoint_address;

				found_endpoint = PU_TRUE;

				break;
			}
		}
	}

	libusb_detach_kernel_driver(context->libusb_printer_handle, interface_number);

	libusb_claim_interface(context->libusb_printer_handle, interface_number);

	result = libusb_bulk_transfer(context->libusb_printer_handle, endpoint_address, data, sizeof(data), &bytes_transferred, 0);

	if(result < 0) {
		printf("libusb error! failed to perform bulk transfer: %s\n", libusb_error_name(result));
	}

	libusb_release_interface(context->libusb_printer_handle, interface_number);

	libusb_attach_kernel_driver(context->libusb_printer_handle, interface_number);

	pu_cleanup(context);

	return PU_SUCCESS;
}

