#ifdef LIBUSB_BACKEND

#include <inttypes.h>

#include "./libusb.h"

typedef struct device_data_t {
	struct libusb_data_t {
		uint8_t interface;
		uint8_t endpoint;
	} libusb_data;
} device_data_t;

#endif
