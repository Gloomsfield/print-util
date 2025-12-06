#pragma once

#include <inttypes.h>

#include "statuses.h"

typedef uint32_t printer_handle_t;

PU_STATUS_T pu_print_markdown(printer_handle_t printer_handle, char * markdown);

