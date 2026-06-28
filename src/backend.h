#pragma once

#ifndef LIBUSB_BACKEND
#error at least one backend must be `#define`d!
#endif

#ifdef LIBUSB_BACKEND
#include "backends/libusb.h"
#endif
