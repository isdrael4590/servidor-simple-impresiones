#pragma once

#include <windows.h>
#include <string>

#include <cpprest/http_listener.h>
#include <cpprest/json.h>
#include <cpprest/uri.h>
#include <cpprest/asyncrt_utils.h>
#include <cpprest/filestream.h>

#include "check_printer.h"

using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;
using namespace utility;

#pragma comment(lib, "setupapi.lib")
#pragma comment(lib, "gdiplus.lib")

using namespace Gdiplus;

void handle_print_request(http_request request);
bool printImageToPrinter(const std::string& printerName, const std::string& imagePath);

const std::string nombre_archivo = "output.png";

// Initialize GDI+
class GdiplusInitializer {
public:
    GdiplusInitializer() {
        GdiplusStartupInput gdiplusStartupInput;
        GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);
    }
    ~GdiplusInitializer() {
        GdiplusShutdown(gdiplusToken);
    }
private:
    ULONG_PTR gdiplusToken;
};

class Service {
public:
    static void WINAPI ServiceMain(DWORD argc, LPTSTR* argv);
    static void WINAPI ServiceCtrlHandler(DWORD ctrlCode);

private:
    static SERVICE_STATUS ServiceStatus;
    static SERVICE_STATUS_HANDLE ServiceStatusHandle;

    static bool Running;

    static void ReportServiceStatus(DWORD currentState, DWORD win32ExitCode, DWORD waitHint);
    static void InitService();

    static void StartHttpServer();
};


