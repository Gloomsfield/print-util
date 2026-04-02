#pragma once

#include "print-util_status.h"

#include <inttypes.h>

typedef struct pu_databuffer_t pu_databuffer_t;

pu_status_t pu_peek_databuffer_32(pu_databuffer_t databuffer, uint32_t* out);
pu_status_t pu_peek_databuffer_16(pu_databuffer_t databuffer, uint16_t* out);
pu_status_t pu_peek_databuffer_8(pu_databuffer_t databuffer, uint8_t* out);

pu_status_t pu_consume_databuffer_32(pu_databuffer_t databuffer, uint32_t* out);
pu_status_t pu_consume_databuffer_16(pu_databuffer_t databuffer, uint16_t* out);
pu_status_t pu_consume_databuffer_8(pu_databuffer_t databuffer, uint8_t* out);

