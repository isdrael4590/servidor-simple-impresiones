#include "pch.h"
#include "main.h"
#include "logger.h"
#include "check_printer.h"
#include "utils.h"

using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;
using namespace utility;

// Directorio base para guardar los archivos recibidos
const std::wstring BASE_DIR = L"etiquetas"; 

void handle_print_request(http_request request) {
	auto tipo_dato = request.headers().content_type();
	if (tipo_dato != U("text/plain"))
		spdlog::debug("Recibiendo una cadena de caracteres plana");
	else if(tipo_dato.find(U("multipart/form-data")) != utility::string_t::npos)
		spdlog::debug("Recibiendo un archivo desde una forma");
	else{
		request.reply(status_codes::NotImplemented, U("Tipo de dato no implementado aún, por favor requiera asistencia"));
		spdlog::error("Tipo de dato no implementado " + utility::conversions::to_utf8string(tipo_dato) + " requiera asistencia");
		return;
	}

	std::string nombre_impresora = findZebraPrinter();
	if (nombre_impresora.empty()) {
		request.reply(status_codes::ExpectationFailed, U("La impresora no está lista para imprimir o no está conectada"));
		return;
	}

	// Crear subcarpetas con la fecha actual
	std::wstring timeFolder = get_current_time_folder();
	std::wstring folderPath = BASE_DIR + L"\\" + timeFolder;
	std::filesystem::create_directory(folderPath);
	spdlog::info("Archivo recibido va a ser guardado en la siguiente carpeta: {}", std::filesystem::path(folderPath).string());
	std::wstring filePath = folderPath + L"\\uploaded_file.pdf";
	
	auto fileStream = std::make_shared<concurrency::streams::ostream>();

	pplx::task<void> requestTask = concurrency::streams::fstream::open_ostream(filePath)
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

					std::vector<std::string> imagePaths;
					convert_pdf_to_images(filePath, imagePaths);
					// Imprimir cada imagen
					for (const auto& imagePath : imagePaths)
					{
						printImageToPrinter(nombre_impresora, imagePath);
					}
					request.reply(status_codes::OK, U("Archivo guardado correctamente"));
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
	if (!std::filesystem::exists(BASE_DIR))
	{
		spdlog::info("Creando el directorio base para guardar las imágenes: {}", std::filesystem::path(BASE_DIR).string());
		std::filesystem::create_directory(BASE_DIR);
	}

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