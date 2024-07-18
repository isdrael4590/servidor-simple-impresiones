#include "pch.h"
#include "check_printer.h"

// Function to find the Zebra ZD421 printer
std::string findZebraPrinter() {
	DWORD dwNeeded, dwReturned;
	PRINTER_INFO_2* pinfo = NULL;

	EnumPrinters(PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS, NULL, 2, NULL, 0, &dwNeeded, &dwReturned);

	pinfo = (PRINTER_INFO_2*)malloc(dwNeeded);
	if (!pinfo) {
		spdlog::error("Fallo alocación de memoria");
		return "";
	}

	if (!EnumPrinters(PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS, NULL, 2, (LPBYTE)pinfo, dwNeeded, &dwNeeded, &dwReturned)) {
		spdlog::error("Fallo en la enumeración de impresoras");
		free(pinfo);
		return "";
	}
	// TODO: Añadir soporte para más impresoras
	for (DWORD i = 0; i < dwReturned; ++i) {
		if (std::string(pinfo[i].pDriverName).find("ZDesigner") != std::string::npos &&
			std::string(pinfo[i].pPrinterName).find("ZD421") != std::string::npos) {
			std::string printerName = pinfo[i].pPrinterName;
			free(pinfo);
			spdlog::info("Zebra ZD421 esta conectada");
			return printerName;
		}
	}

	free(pinfo);
	return "";
}