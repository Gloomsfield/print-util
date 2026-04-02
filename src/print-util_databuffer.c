#include "print-util_databuffer.h"
#include "print-util_status.h"

struct pu_databuffer_t {
	const uint8_t* data;
	uint32_t length;
};

pu_status_t pu_peek_databuffer_32(pu_databuffer_t databuffer, uint32_t* out) {
	if(databuffer.length < 4) { pu_propagate(PU_DATABUFFER_TOOSHORT, PU_ERROR); }

	uint32_t data = 0;

	data |= databuffer.data[0] << 24;
	data |= databuffer.data[1] << 16;
	data |= databuffer.data[2] << 8;
	data |= databuffer.data[3] << 0;

	*out = data;

	return PU_SUCCESS;
}

pu_status_t pu_peek_databuffer_16(pu_databuffer_t databuffer, uint16_t* out) {
	if(databuffer.length < 2) { pu_propagate(PU_DATABUFFER_TOOSHORT, PU_ERROR); }

	uint16_t data = 0;

	data |= databuffer.data[0] << 8;
	data |= databuffer.data[1] << 0;

	*out = data;

	return PU_SUCCESS;
}

pu_status_t pu_peek_databuffer_8(pu_databuffer_t databuffer, uint8_t* out) {
	if(!databuffer.length) { pu_propagate(PU_DATABUFFER_TOOSHORT, PU_ERROR); }

	*out = databuffer.data[0];

	return PU_SUCCESS;
}

pu_status_t pu_consume_databuffer_32(pu_databuffer_t databuffer, uint32_t* out) {
	if(databuffer.length < 4) { pu_propagate(PU_DATABUFFER_TOOSHORT, PU_ERROR); }

	pu_peek_databuffer_32(databuffer, out);

	databuffer.data += 4;
	databuffer.length -= 4;

	return PU_SUCCESS;
}

pu_status_t pu_consume_databuffer_16(pu_databuffer_t databuffer, uint16_t* out) {
	if(databuffer.length < 2) { pu_propagate(PU_DATABUFFER_TOOSHORT, PU_ERROR); }

	pu_peek_databuffer_16(databuffer, out);

	databuffer.data += 2;
	databuffer.length -= 2;

	return PU_SUCCESS;
}

pu_status_t pu_consume_databuffer_8(pu_databuffer_t databuffer, uint8_t* out) {
	if(!databuffer.length) { pu_propagate(PU_DATABUFFER_TOOSHORT, PU_ERROR); }

	pu_peek_databuffer_8(databuffer, out);

	databuffer.data++;
	databuffer.length--;

	return PU_SUCCESS;
}

