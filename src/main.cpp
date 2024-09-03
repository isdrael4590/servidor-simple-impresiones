#include "pch.h"
#include "main.h"
#include "logger.h"
#include "check_printer.h"
#include <spdlog/spdlog.h>

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
	spdlog::info("newImageWidth: {}, newImageHeight: {}, xOffset: {}, yOffset: {}", newImageWidth, newImageHeight, xOffset, yOffset);

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
	// Create a file to save the uploaded file
	auto fileStream = std::make_shared<concurrency::streams::ostream>();

	// Open a file stream to write the file
	pplx::task<void> requestTask = concurrency::streams::fstream::open_ostream(U("uploaded_file.pdf"))
		.then([=](concurrency::streams::ostream outFile)
			{
				*fileStream = outFile;

				// Read the request body (which should contain the file data)
				return request.body().read_to_end(fileStream->streambuf());
			})
		.then([=](size_t)
			{
				return fileStream->close();
			})
		.then([=](pplx::task<void> previousTask)
			{
				try
				{
					previousTask.get();
					spdlog::info("File saved successfully");
					request.reply(status_codes::OK, U("File saved successfully"));
				}
				catch (const std::exception& e)
				{
					request.reply(status_codes::InternalError, U("Failed to save the file"));
					spdlog::info("Failed to save the file");
					std::wcout << L"Error: " << e.what() << std::endl;
				}
			});

	try
	{
		requestTask.wait();
	}
	catch (const std::exception& e)
	{
		std::wcout << L"Error: " << e.what() << std::endl; // TODO: Revisar error aquí
	}
}

int main() {
	Logger::init();
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

		std::string line;
		std::getline(std::cin, line); // Wait for user input to keep the server running
	}
	catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
	}

	spdlog::info("Servidor finalizado");
	return 0;
}