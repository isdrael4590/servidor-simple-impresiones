#include "pch.h"
#include "main.h"
#include "logger.h"
#include "check_printer.h"

using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;
using namespace utility;

#pragma comment(lib, "setupapi.lib")
#pragma comment(lib, "gdiplus.lib")

using namespace Gdiplus;

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

const std::string nombre_archivo = "output.png";

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
	Graphics* graphics = new Graphics(hdc);
	Image image(std::wstring(imagePath.begin(), imagePath.end()).c_str());

	if (graphics->DrawImage(&image, 0, 0) != Ok) {
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
			std::cout << nombre_impresora << std::endl;
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

int main() {
	GdiplusInitializer gdiplusInitializer;
	rotate_spdlog();
	uri_builder uri(U("http://*:3000")); // Listen on all available network interfaces
	auto addr = uri.to_uri().to_string();
	http_listener listener(addr);
	listener.support(methods::POST, handle_print_request);

	try {
		listener
			.open()
			.then([&listener]() { spdlog::info("Iniciando servidor en la dirección: {}", utility::conversions::to_utf8string(listener.uri().to_string())); })
			.wait();

		std::string line;
		std::getline(std::cin, line); // Wait for user input to keep the server running
	}
	catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
	}

	spdlog::info("Servidor finalizado");
	return 0;
}