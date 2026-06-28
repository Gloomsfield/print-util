#pragma once

#include <inttypes.h>

typedef struct device_data_t device_data_t;

typedef struct request_data_t {
	char* string;
	device_data_t* device;
} request_data_t;

typedef struct encoded_data_t {
	uint8_t* data;
	uint32_t length;
} encoded_data_t;

typedef struct encoding_scheme_t {
	int(* encode)(char*, encoded_data_t*);
} encoding_scheme_t;

typedef struct sending_scheme_t {
	int(* send)(encoded_data_t);
} sending_scheme_t;
