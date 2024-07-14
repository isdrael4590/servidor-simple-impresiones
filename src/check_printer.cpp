#include "pch.h"
#include "check_printer.h"

bool is_zebra_connected() {
	libusb_context* ctx = nullptr;
	libusb_device** devs;
	ssize_t cnt;
	int r;

	r = libusb_init(&ctx);
	if (r < 0) {
		std::cerr << "Init Error " << r << std::endl;
		return 1;
	}

	cnt = libusb_get_device_list(ctx, &devs);
	if (cnt < 0) {
		std::cerr << "Get Device Error" << std::endl;
		libusb_exit(ctx);
		return 1;
	}

	libusb_device* device;
	int i = 0;
	bool zebraConnected = false;

	while ((device = devs[i++]) != nullptr) {
		libusb_device_descriptor desc;
		r = libusb_get_device_descriptor(device, &desc);
		if (r < 0) {
			std::cerr << "Failed to get device descriptor" << std::endl;
			libusb_free_device_list(devs, 1);
			libusb_exit(ctx);
			return 1;
		}

		// Zebra ZD421 typically has Vendor ID: 0x0A5F, Product ID: 0x0185
		if (desc.idVendor == 0x0A5F && desc.idProduct == 0x0185) {
			zebraConnected = true;
			break;
		}
	}

	libusb_free_device_list(devs, 1);
	libusb_exit(ctx);

	if (zebraConnected)
		spdlog::info("Zebra ZD421 esta conectada");
	else
		spdlog::info("Zebra ZD421 no esta conectada");
	return zebraConnected;
}