#include <cpprest/http_listener.h>
#include <cpprest/uri.h>
#include <cpprest/json.h>
#include <windows.h>
#include <string>
#include <iostream>

using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;

std::string wstring_to_string(const std::wstring& wstr) {
    int bufferSize = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
    std::string str(bufferSize, 0);
    WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, &str[0], bufferSize, NULL, NULL);
    return str;
}

void handle_print_request(http_request request) {
    auto query = uri::split_query(request.request_uri().query());
    std::cout << "Imprimiendo Query" << query.size() << " body: " << request.body() << " headers: " << std::endl;
    for (auto i = request.headers().begin(); i != request.headers().end(); i++) {
        std::cout << utility::conversions::to_utf8string(i->first) << " \t\t\t" << utility::conversions::to_utf8string(i->second) << std::endl;
    }
    auto it = query.find(U("image"));
    if (it == query.end()) {
        request.reply(status_codes::BadRequest, "No image specified");
        std::cout << "Attempting to connecti without submitting an image" << std::endl;
        return;
    }

    std::wstring image_path_w = it->second;
    std::string image_path = wstring_to_string(image_path_w);
    LPCSTR image_path_lpcstr = image_path.c_str();

    try {
        // Print the image
        HINSTANCE result = ShellExecuteA(NULL, "print", image_path_lpcstr, NULL, NULL, SW_HIDE);
        if ((int)result <= 32) {
            throw std::runtime_error("Failed to print");
        }

        request.reply(status_codes::OK, "Print job sent successfully");
    }
    catch (const std::exception& e) {
        request.reply(status_codes::InternalError, e.what());
    }
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