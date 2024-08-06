#include "Service.h"
#include "logger.h"

SERVICE_STATUS Service::ServiceStatus = { 0 };
SERVICE_STATUS_HANDLE Service::ServiceStatusHandle = nullptr;
bool Service::Running = false;

void WINAPI Service::ServiceMain(DWORD argc, LPTSTR* argv) {
    ServiceStatusHandle = RegisterServiceCtrlHandler("PrinterService", ServiceCtrlHandler);
    if (!ServiceStatusHandle) {
        Logger::init();
        spdlog::error("RegisterServiceCtrlHandler failed");
        return;
    }

    ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    ServiceStatus.dwServiceSpecificExitCode = 0;

    ReportServiceStatus(SERVICE_START_PENDING, NO_ERROR, 3000);
    InitService();
}

void WINAPI Service::ServiceCtrlHandler(DWORD ctrlCode) {
    switch (ctrlCode) {
    case SERVICE_CONTROL_STOP:
        ReportServiceStatus(SERVICE_STOP_PENDING, NO_ERROR, 0);
        Running = false;
        ReportServiceStatus(SERVICE_STOPPED, NO_ERROR, 0);
        break;
    default:
        break;
    }
}

void Service::ReportServiceStatus(DWORD currentState, DWORD win32ExitCode, DWORD waitHint) {
    ServiceStatus.dwCurrentState = currentState;
    ServiceStatus.dwWin32ExitCode = win32ExitCode;
    ServiceStatus.dwWaitHint = waitHint;

    if (currentState == SERVICE_START_PENDING) {
        ServiceStatus.dwControlsAccepted = 0;
    }
    else {
        ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    }

    SetServiceStatus(ServiceStatusHandle, &ServiceStatus);
}

void Service::InitService() {
    ReportServiceStatus(SERVICE_RUNNING, NO_ERROR, 0);
    Running = true;

    Logger::init();
    GdiplusInitializer gdiplusInitializer;

    while (Running) {
        std::string printerName = findZebraPrinter();
        if (!printerName.empty()) {
            std::string imagePath = "path_to_image.png";
            if (printImageToPrinter(printerName, imagePath)) {
                spdlog::info("Image is sent to printer successfully");
            }
            else {
                spdlog::error("Failed to print image to Zebra ZD421");
            }
        }
        else {
            spdlog::info("Zebra ZD421 is not connected");
        }
        Sleep(60000);  // Check every minute
    }
}

void Service::StartHttpServer()
{
    GdiplusInitializer gdiplusInitializer;
    uri_builder uri(U("http://*:3000")); // Listen on all available network interfaces
    auto addr = uri.to_uri().to_string();
    http_listener listener(addr);
    listener.support(methods::POST, handle_print_request);

    try {
        listener
            .open()
            .then([&listener]() { spdlog::info("Iniciando servidor en la dirección: {}", utility::conversions::to_utf8string(listener.uri().to_string())); })
            .wait();

    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    spdlog::info("Servidor finalizado");
}

// Function to send image to the printer
bool printImageToPrinter(const std::string& printerName, const std::string& imagePath) {
    // Initialize GDI+.
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    HANDLE hPrinter;
    DWORD dwBytesWritten;

    if (!OpenPrinter(const_cast<LPSTR>(printerName.c_str()), &hPrinter, NULL)) {
        std::cerr << "Failed to open printer: " << printerName << std::endl;
        return false;
    }

    DOCINFO docInfo;
    ZeroMemory(&docInfo, sizeof(docInfo));
    docInfo.cbSize = sizeof(docInfo);
    docInfo.lpszDocName = "GdiplusPrint";

    HDC hdc = CreateDC(NULL, printerName.c_str(), NULL, NULL);
    if (!hdc) {
        spdlog::error("Fallo en crear el contexto de dispositivo para la impresora: {}", printerName);
        EndPagePrinter(hPrinter);
        EndDocPrinter(hPrinter);
        ClosePrinter(hPrinter);
        return false;
    }

    if (StartDoc(hdc, &docInfo) == 0) {
        spdlog::error("Fallo al iniciar el documento en la impresora: {}", printerName);
        ClosePrinter(hPrinter);
        return false;
    }
    StartPage(hdc);
    // Get the printable area of the printer
    int printerWidth = GetDeviceCaps(hdc, HORZRES);
    int printerHeight = GetDeviceCaps(hdc, VERTRES);
    Graphics* graphics = new Graphics(hdc);
    graphics->SetPageUnit(UnitPixel);
    graphics->SetPageScale(1.0);  // Force to t
    Image image(std::wstring(imagePath.begin(), imagePath.end()).c_str());

    // Get the size of the image
    int imageWidth = image.GetWidth();
    int imageHeight = image.GetHeight();
    // Calculate the aspect ratios
    double printerAspectRatio = static_cast<double>(printerWidth) / static_cast<double>(printerHeight);
    double imageAspectRatio = static_cast<double>(imageWidth) / static_cast<double>(imageHeight);

    // Determine the scaling factors
    double scaleFactor;
    if (imageAspectRatio > printerAspectRatio) {
        scaleFactor = static_cast<double>(printerWidth) / static_cast<double>(imageWidth);
    }
    else {
        scaleFactor = static_cast<double>(printerHeight) / static_cast<double>(imageHeight);
    }

    // Calculate the new dimensions of the image
    int newImageWidth = static_cast<int>(imageWidth * scaleFactor);
    int newImageHeight = static_cast<int>(imageHeight * scaleFactor);
    // Calculate the position to center the image on the page
    int xOffset = (printerWidth - newImageWidth) / 2;
    int yOffset = (printerHeight - newImageHeight) / 2;

    if (graphics->DrawImage(&image, xOffset, yOffset, newImageWidth, newImageHeight) != Ok) {
        spdlog::error("Failed to draw image on printer: {}", printerName);
        DeleteDC(hdc);
        EndPage(hdc);
        EndDoc(hdc);
        ClosePrinter(hPrinter);
        delete(graphics);
        return false;
    }
    delete(graphics);
    EndPage(hdc);
    EndDoc(hdc);
    DeleteDC(hdc);
    ClosePrinter(hPrinter);
    GdiplusShutdown(gdiplusToken);


    return true;
}

void handle_print_request(http_request request) {
    if (request.headers().content_type() != U("text/plain")) {
        request.reply(status_codes::NotImplemented, U("Tipo de dato no implementado aún, por favor requiera asistencia"));
    }
    request.extract_string().then([=](utility::string_t base64_string) {
        try {
            // Decodificar la cadena en base64
            auto decoded_data = utility::conversions::from_base64(base64_string);
            // Convertir a un vector de bytes
            std::vector<unsigned char> byte_array(decoded_data.begin(), decoded_data.end());

            // Guardar el archivo
            // TODO: Guardar con el tipo y nombre de dato especificado desde PHP
            std::ofstream out_file(nombre_archivo, std::ios::binary);
            out_file.write(reinterpret_cast<const char*>(byte_array.data()), byte_array.size());
            out_file.close();

            spdlog::info("Imagen guardada como {}", nombre_archivo);
            std::string nombre_impresora = findZebraPrinter();
            if (!nombre_impresora.empty()) {
                bool status = printImageToPrinter(nombre_impresora, nombre_archivo);
                if (status) {
                    //Elimina la imagen
                    if (std::filesystem::remove(nombre_archivo))
                        spdlog::warn("Imagen {} eliminada.", nombre_archivo);
                    else
                        spdlog::warn("Imagen {} no eliminada ", nombre_archivo);
                    request.reply(status_codes::OK, U("Imagen correctamente recibida e impresa."));
                }
                else {
                    request.reply(status_codes::OK, U("Imagen correctamente recibida, guardada pero tiene que imprimir manualmente."));
                }
            }
            else
                request.reply(status_codes::ExpectationFailed, U("Impresora Zebra no conectada."));

        }
        catch (const std::exception& e) {
            ucout << U("Error decodificando la cadena caracteres: ") << e.what() << std::endl;
            request.reply(status_codes::BadRequest, U("Cadena de caracteres de base64 invalida."));
        }
        }).then([](pplx::task<void> t) {
            try {
                t.get();
            }
            catch (const std::exception& e) {
                ucout << U("Error externo: ") << e.what() << std::endl;
            }
            });
}