#include <cpprest/http_listener.h>
#include <cpprest/json.h>
#include <cpprest/uri.h>
#include <cpprest/asyncrt_utils.h>
#include <cpprest/filestream.h>
#include <iostream>
#include <vector>
#include <string>

using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;
using namespace utility;

// Function to decode base64 string
std::string base64_decode(const std::string& in) {
	std::string out;
	std::vector<int> T(256, -1);
	for (int i = 0; i < 64; i++) T["ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[i]] = i;

	int val = 0, valb = -8;
	for (unsigned char c : in) {
		if (T[c] == -1) break;
		val = (val << 6) + T[c];
		valb += 6;
		if (valb >= 0) {
			out.push_back(char((val >> valb) & 0xFF));
			valb -= 8;
		}
	}
	return out;
}
void handle_print_request(http_request request) {
	request.extract_string().then([=](utility::string_t base64_string) {
		try {
			// Decode base64 string
			auto decoded_data = utility::conversions::from_base64(base64_string);
			// Convert decoded data to a vector of bytes
			std::vector<unsigned char> byte_array(decoded_data.begin(), decoded_data.end());

			// Save to file
			std::ofstream out_file("output.png", std::ios::binary);
			out_file.write(reinterpret_cast<const char*>(byte_array.data()), byte_array.size());
			out_file.close();

			ucout << U("Image saved as output.png") << std::endl;
			system("mspaint /pt output.png");
			request.reply(status_codes::OK, U("Imagen correctamente recibida y guardada."));
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
				ucout << U("Error: ") << e.what() << std::endl;
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
			.then([&listener]() { std::wcout << L"Starting server at: " << listener.uri().to_string() << std::endl; })
			.wait();

		std::string line;
		std::getline(std::cin, line); // Wait for user input to keep the server running
	}
	catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
	}

	return 0;
}