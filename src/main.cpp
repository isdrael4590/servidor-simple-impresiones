#include "pch.h"
#include "main.h"
#include "logger.h"
#include <spdlog/spdlog.h>
#include <Windows.h>

void InstallService() {
	SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
	if (!hSCManager) {
		std::cerr << "OpenSCManager failed" << std::endl;
		return;
	}

	SC_HANDLE hService = CreateService(
		hSCManager,
		"PrinterService",
		"PrinterService",
		SERVICE_ALL_ACCESS,
		SERVICE_WIN32_OWN_PROCESS,
		SERVICE_AUTO_START,
		SERVICE_ERROR_NORMAL,
		"C:\\Users\\josejacomeb\\Documents\\GitHub\\servidor-simple-impresiones\\out\\install\\x64-debug-ms\\bin\\servidor-simple-impresiones.exe",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL);

	if (!hService) {
		std::cerr << "CreateService failed" << std::endl;
		CloseServiceHandle(hSCManager);
		return;
	}

	CloseServiceHandle(hService);
	CloseServiceHandle(hSCManager);
}

void RemoveService() {
	SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
	if (!hSCManager) {
		std::cerr << "OpenSCManager failed" << std::endl;
		return;
	}

	SC_HANDLE hService = OpenService(hSCManager, "PrinterService", SERVICE_STOP | DELETE);
	if (!hService) {
		std::cerr << "OpenService failed" << std::endl;
		CloseServiceHandle(hSCManager);
		return;
	}

	DeleteService(hService);

	CloseServiceHandle(hService);
	CloseServiceHandle(hSCManager);
}

int main(int argc, char* argv[]) {
	if (argc > 1) {
		std::cout << argv[1] << std::endl;
		if (strcmp(argv[1], "install") == 0) {
			InstallService();
			return 0;
		}
		else if (strcmp(argv[1], "remove") == 0) {
			RemoveService();
			return 0;
		}
	}

	SERVICE_TABLE_ENTRY ServiceTable[] = {
		{ const_cast<LPSTR>("PrinterService"), (LPSERVICE_MAIN_FUNCTION)Service::ServiceMain },
		{ NULL, NULL }
	};

	if (!StartServiceCtrlDispatcher(ServiceTable)) {
		Logger::init();
		spdlog::error("StartServiceCtrlDispatcher failed");
	}

	return 0;
}