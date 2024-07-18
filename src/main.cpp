#include "pch.h"
#include "main.h"
#include "logger.h"
#include "check_printer.h"

using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;
using namespace utility;

const std::string nombre_archivo = "output.png";

libusb_device_handle* handle = nullptr;


// Function to encode image to ZPL
std::string imageToZpl(const cv::Mat& img) {
	std::ostringstream zpl;
	int width = img.cols;
	int height = img.rows;
	zpl << "^XA^FO50,50^GFA," << width * height / 8 << "," << width * height / 8 << "," << width / 8 << ",";

	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; x += 8) {
			unsigned char byte = 0;
			for (int bit = 0; bit < 8; ++bit) {
				if (x + bit < width) {
					unsigned char pixel = img.at<uchar>(y, x + bit);
					if (pixel < 128) {
						byte |= (1 << (7 - bit));
					}
				}
			}
			zpl << std::hex << std::uppercase << (byte < 16 ? "0" : "") << (int)byte;
		}
	}

	zpl << "^FS^XZ";
	return zpl.str();
}

// Function to send ZPL to the printer
bool printZplToPrinter(const std::string& printerName, const std::string& zpl) {
	HANDLE hPrinter;
	DOC_INFO_1 docInfo;
	DWORD dwBytesWritten;

	if (!OpenPrinter(const_cast<LPSTR>(printerName.c_str()), &hPrinter, NULL)) {
		spdlog::error("Failed to open printer: {}",printerName);
		return false;
	}

	docInfo.pDocName = const_cast<LPSTR>("ZPL Document");
	docInfo.pOutputFile = NULL;
	docInfo.pDatatype = const_cast<LPSTR>("RAW");

	if (StartDocPrinter(hPrinter, 1, (LPBYTE)&docInfo) == 0) {
		spdlog::error("Failed to start document on printer: {}", printerName);
		ClosePrinter(hPrinter);
		return false;
	}

	if (StartPagePrinter(hPrinter) == 0) {
		spdlog::error("Failed to start page on printer: {}", printerName);
		EndDocPrinter(hPrinter);
		ClosePrinter(hPrinter);
		return false;
	}

	if (!WritePrinter(hPrinter, (LPVOID)zpl.c_str(), zpl.size(), &dwBytesWritten)) {
		spdlog::error("Failed to write to printer: {}", printerName);
		EndPagePrinter(hPrinter);
		EndDocPrinter(hPrinter);
		ClosePrinter(hPrinter);
		return false;
	}

	EndPagePrinter(hPrinter);
	EndDocPrinter(hPrinter);
	ClosePrinter(hPrinter);

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
				// Imprime imagen 
				cv::Mat img = cv::imread(nombre_archivo, cv::IMREAD_GRAYSCALE);
				if (img.empty()) {
					spdlog::warn("Error al leer la imágen");
				}

				std::string zpl = imageToZpl(img);
				bool status = printZplToPrinter(nombre_impresora, zpl);
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