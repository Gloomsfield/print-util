#pragma once

#include "../types/types.h"

int determine_encoding_scheme(request_data_t request_data, encoding_scheme_t* encoding_scheme);
int determine_sending_scheme(request_data_t request_data, sending_scheme_t* sending_scheme);
