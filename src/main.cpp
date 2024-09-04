#include "pch.h"
#include "main.h"
#include "logger.h"
#include "check_printer.h"
#include "utils.h"

using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;
using namespace utility;


void handle_print_request(http_request request) {
	if (request.headers().content_type() != U("text/plain")) {
		request.reply(status_codes::NotImplemented, U("Tipo de dato no implementado aún, por favor requiera asistencia"));
	}
	std::wcout << L"Received POST request" << std::endl;

	auto fileStream = std::make_shared<concurrency::streams::ostream>();

	pplx::task<void> requestTask = concurrency::streams::fstream::open_ostream(U("uploaded_file.pdf"))
		.then([=](concurrency::streams::ostream outFile)
			{
				*fileStream = outFile;

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

					// Print the PDF file
					std::wstring filePath = L"uploaded_file.pdf";
					std::string nombre_impresora = findZebraPrinter();
					if (!nombre_impresora.empty()) {
						std::vector<std::string> imagePaths;
						convert_pdf_to_images(filePath, imagePaths);
						// Print each image
						for (const auto& imagePath : imagePaths)
						{
							printImageToPrinter(nombre_impresora, imagePath);
						}
						request.reply(status_codes::OK, U("Archivo guardado correctamente"));
					}
					else
						request.reply(status_codes::ExpectationFailed, U("Impresora Zebra no conectada."));
				}
				catch (const std::exception& e)
				{
					request.reply(status_codes::InternalError, U("Failed to save the file"));
					std::wcout << L"Error: " << e.what() << std::endl;
				}
			});

	try
	{
		requestTask.wait();
	}
	catch (const std::exception& e)
	{
		std::wcout << L"Error: " << e.what() << std::endl;
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