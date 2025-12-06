#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <getopt.h>

#include <libusb-1.0/libusb.h>
#include <systemd/sd-device.h>

#include "pu.h"
#include "pu-constants.h"

static pu_context_t global_context;

PU_STATUS_T pu_init_libusb(libusb_context ** libusb_ctx) {
	int libusb_init_result = libusb_init(libusb_ctx);

	if(libusb_init_result) {
		printf("libusb error! %s\n", libusb_error_name(libusb_init_result));
		return PU_FAILURE;
	}

	return PU_SUCCESS;
}

PU_STATUS_T pu_get_usb_devices() {
	int result = libusb_get_device_list(global_context.libusb_ctx, &global_context.libusb_devices);

	if(result < 0) {
		printf("libusb error! failed to get device list: %s\n", libusb_error_name(result));
		return PU_FAILURE;
	}

	uint32_t libusb_device_count = result;

	sd_device_enumerator * sd_enum;

	result = sd_device_enumerator_new(&sd_enum);

	if(result < 0) {
		printf("sd-device error! failed to generate enumerator: %s\n", strerror(-result));
		return PU_FAILURE;
	}

	result = sd_device_enumerator_add_match_subsystem(sd_enum, "usb", 1);
	result = sd_device_enumerator_add_match_property(sd_enum, "DEVTYPE", "usb_device");
	
	if(result < 0) {
		printf("sd-device error! failed to add matches: %s\n", strerror(-result));
		return PU_FAILURE;
	}
	
	sd_device * device = sd_device_enumerator_get_device_first(sd_enum);

	const char * dev_model_buffer;
	const char * dev_vendor_id_buffer;
	const char * dev_product_id_buffer;

	uint32_t i = 0;
	
	while(device != NULL) {
		if(i >= PU_DEVICE_BUFFER_SIZE) {
			printf("print-util error! device buffer too small!\n");

			return PU_FAILURE;
		}

		sd_device_get_property_value(device, "ID_MODEL", &dev_model_buffer);

		sd_device_get_property_value(device, "ID_VENDOR_ID", &dev_vendor_id_buffer);
		sd_device_get_property_value(device, "ID_MODEL_ID", &dev_product_id_buffer);

		global_context.device_buffer[i] = (pu_usb_device_t){
			.name = "UNKNOWN",
			.vendor_id = strtol(dev_vendor_id_buffer, NULL, 16),
			.product_id = strtol(dev_product_id_buffer, NULL, 16),
		};

		memcpy(global_context.device_buffer[i].name, dev_model_buffer, strlen(dev_model_buffer));

		i++;

		device = sd_device_enumerator_get_device_next(sd_enum);
	}

	global_context.device_count = i;

	global_context.libusb_device_count = libusb_device_count;

	sd_device_enumerator_unref(sd_enum);

	return PU_SUCCESS;
}

PU_STATUS_T pu_init(int argc, char * argv[]) {
	if(pu_init_libusb(&global_context.libusb_ctx) != PU_SUCCESS) {
		printf("libusb error! failed to init libusb!");
		return PU_FAILURE;
	}

	global_context.initialized = 1;
	return PU_SUCCESS;
}

void pu_cleanup() {
	libusb_free_device_list(global_context.libusb_devices, 1);
	libusb_exit(global_context.libusb_ctx);
}

PU_STATUS_T pu_determine_libusb_descriptor_info() {
	struct libusb_config_descriptor * config_descriptor;

	int result = 0;
	result = libusb_get_active_config_descriptor(libusb_get_device(global_context.libusb_printer_handle), &config_descriptor);

	if(result < 0) {
		printf("libusb error! failed to get active config descriptor: %s\n", libusb_error_name(result));

		return PU_FAILURE;
	}

	for(uint32_t i = 0; i < config_descriptor->bNumInterfaces; i++) {
		const struct libusb_interface * interface = &config_descriptor->interface[i];

		for(uint32_t j = 0; j < interface->num_altsetting; j++) {
			const struct libusb_interface_descriptor * altsetting = &interface->altsetting[j];

			for(uint32_t k = 0; k < altsetting->bNumEndpoints; k++) {
				const struct libusb_endpoint_descriptor * endpoint = &altsetting->endpoint[k];

				if((endpoint->bmAttributes & LIBUSB_TRANSFER_TYPE_MASK) != LIBUSB_TRANSFER_TYPE_BULK) {
					continue;
				}

				uint8_t endpoint_address = endpoint->bEndpointAddress;

				if((endpoint_address & LIBUSB_ENDPOINT_DIR_MASK) == LIBUSB_ENDPOINT_IN) {
					continue;
				}

				global_context.libusb_device_interface_number = i;
				global_context.libusb_device_endpoint_address = endpoint_address;

				return PU_SUCCESS;
			}
		}
	}

	printf("print-util error! failed to find suitable USB device!\n");

	return PU_FAILURE;
}

PU_STATUS_T pu_select() {
	if(!global_context.initialized) {
		printf("print-util error! please initialize the context before running it!\n");

		pu_cleanup();

		return PU_FAILURE;
	}

	if(pu_get_usb_devices() != PU_SUCCESS) {
		printf("print-util error! failed to get USB devices!\n");

		pu_cleanup();
		
		return PU_FAILURE;
	}

	if(global_context.device_count == 0) {
		printf("print-util error! no USB devices found!\n");

		pu_cleanup();

		return PU_FAILURE;
	}

	printf("please select your printer from the following list of USB devices:\n");

	for(uint32_t i = 0; i < global_context.device_count; i++) {
		printf("%u - %s\n", i + 1, global_context.device_buffer[i].name);
	}

	uint32_t device_choice = 0;
	int scanf_status = 0;

	while((scanf_status = scanf("%d", &device_choice)) == 1 && device_choice < 1 || global_context.device_count < device_choice) {
		printf("[1, %u]: ", global_context.device_count);
	}

	if(scanf_status != 1) {
		printf("quitting!");

		pu_cleanup();

		return PU_FAILURE;
	}

	global_context.libusb_printer_handle = libusb_open_device_with_vid_pid(
		global_context.libusb_ctx,
		global_context.device_buffer[device_choice - 1].vendor_id,
		global_context.device_buffer[device_choice - 1].product_id
	);

	if(global_context.libusb_printer_handle == NULL) {
		printf("libusb printer handle was NULL!\n");

		pu_cleanup();

		return PU_FAILURE;
	}

	pu_determine_libusb_descriptor_info();

	if(libusb_kernel_driver_active(
		global_context.libusb_printer_handle,
		global_context.libusb_device_interface_number
	)) {
		global_context.kernel_was_active = 1;

		libusb_detach_kernel_driver(
			global_context.libusb_printer_handle,
			global_context.libusb_device_interface_number
		);
	}

	libusb_claim_interface(
		global_context.libusb_printer_handle,
		global_context.libusb_device_interface_number
	);

	return PU_SUCCESS;
}

PU_STATUS_T pu_send(const uint8_t * data, const uint32_t data_length) {
	int result = 0;
	int sent_byte_count = 0;

	result = libusb_bulk_transfer(
		global_context.libusb_printer_handle,
		global_context.libusb_device_endpoint_address,
		(uint8_t *)data,
		data_length,
		&sent_byte_count,
		0
	);

	if(result < 0) {
		printf("libusb error! failed to perform bulk transfer: %s\n", libusb_error_name(result));

		return PU_FAILURE;
	}

	if(sent_byte_count != data_length) {
		printf("print-util error! sent byte count mismatch!\n");

		return PU_FAILURE;
	}

	return PU_SUCCESS;
}

PU_STATUS_T pu_print_text(pu_text_t * text) {
	const uint8_t text_setup_commands[] = {
		// setting font
		PU_ESC,
		'M',
		text->font_index,

		// setting justification
		PU_ESC,
		'a',
		text->justification,

		// setting character size
		0x1d,
		'!',
		(text->character_width - 1) << 3 & (text->character_height - 1),
	};

	const uint8_t end_cut_command[] = {
		0x0a,
		0x1d,
		'V',
		text->cutmode == PU_CUTMODE_FULL ? 66 : 65,
		0x00,
	};

	const uint8_t end_nocut_command[] = {
		0x0a,
	};

	uint8_t command[2048] = { '\0' };

	memcpy(&(command[0]), text_setup_commands, sizeof(text_setup_commands));
	memcpy(&(command[sizeof(text_setup_commands)]), text->string, strlen(text->string));
	memcpy(
		&(command[sizeof(text_setup_commands) + strlen(text->string) + 1]),
		text->cutmode == PU_CUTMODE_NONE ? end_nocut_command : end_cut_command,
		text->cutmode == PU_CUTMODE_NONE ? sizeof(end_nocut_command) : sizeof(end_cut_command)
	);

	pu_send(command, sizeof(text_setup_commands) + strlen(text->string) + 1 + (text->cutmode == PU_CUTMODE_NONE ? sizeof(end_nocut_command) : sizeof(end_cut_command)));

	return PU_SUCCESS;
}

PU_STATUS_T pu_print_image(pu_image_t * image) {
	const uint8_t pu_printcommand_begin_bit_image_mode[] = { 
		PU_ESC,
		'*',
		33,
		image->width & 0xff, // low byte
		image->width >> 8 // high byte
	};

	const uint8_t cut_command[] = {
		0x0a,
		0x1d,
		'V',
		image->cutmode == PU_CUTMODE_FULL ? 66 : 65,
	};

	const uint8_t end_nocut_command[] = {
		0x0a,
	};
	
	const uint8_t advance_paper[] = { PU_ESC, 'J', 1 };

	for(uint32_t i = 0; i < image->height / 24; i++) {
		pu_send(
			pu_printcommand_begin_bit_image_mode,
			sizeof(pu_printcommand_begin_bit_image_mode)
		);

		for(uint32_t x = 0; x < image->width; x++) {
			uint8_t byte_buffer[3] = { 0 };

			for(uint32_t j = 0; j < 3; j++) {
				for(uint32_t y = (i * 24) + (j * 8); y < (i * 24) + ((j + 1) * 8) && y < image->height; y++) {
					uint8_t bit = image->data[(y * image->width + x) / 8];
					bit >>= 7 - (x % 8);
					bit &= 1;
					bit ^= 1;
					bit <<= 7 - (y % 8);

					byte_buffer[j] |= bit;
				}
			}

			pu_send(byte_buffer, 3);
		}

		pu_send(advance_paper, sizeof(advance_paper));
	}

	if(image->cutmode != PU_CUTMODE_NONE) {
		if(pu_send(cut_command, sizeof(cut_command)) != PU_SUCCESS) {
			return PU_FAILURE;
		}
	} else {
		if(pu_send(end_nocut_command, sizeof(end_nocut_command)) != PU_SUCCESS) {
			return PU_FAILURE;
		}
	}

	return PU_SUCCESS;
}

PU_STATUS_T pu_stop() {
	libusb_release_interface(
		global_context.libusb_printer_handle,
		global_context.libusb_device_interface_number
	);

	if(global_context.kernel_was_active) {
		libusb_attach_kernel_driver(
			global_context.libusb_printer_handle,
			global_context.libusb_device_interface_number
		);
	}

	pu_cleanup();

	return PU_SUCCESS;
}

