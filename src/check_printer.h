#pragma once
#include <libusb-1.0/libusb.h>

bool is_zebra_connected(libusb_device_handle* handle);
std::string findZebraPrinter();