#include <cpprest/http_listener.h>
#include <cpprest/json.h>
#include <cpprest/uri.h>
#include <cpprest/asyncrt_utils.h>
#include <cpprest/filestream.h>
#include <iostream>
#include <vector>
#include <string>
#include <filesystem>

using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;
using namespace utility;

const std::string nombre_archivo = "output.png";
const std::string comando = "mspaint /pt " + nombre_archivo;

//TODO: Chequear las impresoras disponibles

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

			std::cout << "Imagen guardada como " << nombre_archivo << std::endl;
			// Imprime imagen 
			int status_code = system(comando.c_str());
			if (status_code == 0) {
				//Elimina la imagen
				if (std::filesystem::remove(nombre_archivo))
					std::cout << "Imagen " << nombre_archivo << " Eliminada.\n";
				else
					std::cout << "Imagen " << nombre_archivo << " no eliminada " << std::endl;
				request.reply(status_codes::OK, U("Imagen correctamente recibida e impresa."));
			}
			else {
				request.reply(status_codes::OK, U("Imagen correctamente recibida, guardada pero tiene que imprimir manualmente."));
			}
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
				ucout << U("Error exterbi: ") << e.what() << std::endl;
			}
			});
}

int main() {
	uri_builder uri(U("http://*:3000")); // Listen on all available network interfaces
	auto addr = uri.to_uri().to_string();
	http_listener listener(addr);
	listener.support(methods::POST, handle_print_request);

	try {
		listener
			.open()
			.then([&listener]() { std::wcout << L"Iniciando servidor en la dirección: " << listener.uri().to_string() << std::endl; })
			.wait();

		std::string line;
		std::getline(std::cin, line); // Wait for user input to keep the server running
	}
	catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
	}

	return 0;
}