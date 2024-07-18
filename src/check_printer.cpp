#include "pch.h"
#include "check_printer.h"

// Function to find the Zebra ZD421 printer
std::string findZebraPrinter() {
	DWORD dwNeeded, dwReturned;
	PRINTER_INFO_2* pinfo = NULL;

	EnumPrinters(PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS, NULL, 2, NULL, 0, &dwNeeded, &dwReturned);

	pinfo = (PRINTER_INFO_2*)malloc(dwNeeded);
	if (!pinfo) {
		std::cerr << "Memory allocation failed." << std::endl;
		return "";
	}

	if (!EnumPrinters(PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS, NULL, 2, (LPBYTE)pinfo, dwNeeded, &dwNeeded, &dwReturned)) {
		std::cerr << "Failed to enumerate printers." << std::endl;
		free(pinfo);
		return "";
	}

	for (DWORD i = 0; i < dwReturned; ++i) {
		spdlog::info("Driver name: " + std::string(pinfo[i].pDriverName));
		spdlog::info("Printer name: " + std::string(pinfo[i].pPrinterName));

		if (std::string(pinfo[i].pDriverName).find("ZDesigner") != std::string::npos &&
			std::string(pinfo[i].pPrinterName).find("ZD421") != std::string::npos) {
			std::string printerName = pinfo[i].pPrinterName;
			free(pinfo);
			return printerName;
		}
	}

	free(pinfo);
	return "";
}

bool is_zebra_connected(libusb_device_handle* printer_handle) {
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
	libusb_device_handle* handle = nullptr;
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
			r = libusb_open(device, &handle);
			if (r != 0) {
				spdlog::debug(libusb_error_name(r));
				spdlog::error("Fallo en abrir la impresora");
				continue;
			}
			zebraConnected = true;
			printer_handle = handle;
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