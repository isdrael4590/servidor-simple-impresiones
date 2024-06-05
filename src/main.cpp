// CMakeProject1.cpp : Defines the entry point for the application.
//

#include "main.h"

using namespace std;
using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;

std::string wstring_to_string(const std::wstring& wstr) {
    int bufferSize = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
    std::string str(bufferSize, 0);
    WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, &str[0], bufferSize, NULL, NULL);
    return str;
}

void handle_print_request(http_request request)
{
    auto query = uri::split_query(request.request_uri().query());
    auto it = query.find(U("image"));
    if (it == query.end())
    {
        request.reply(status_codes::BadRequest, "No image specified");
        return;
    }

    std::wstring image_path_w = it->second;
    std::string image_path = wstring_to_string(image_path_w);
    LPCSTR image_path_lpcstr = image_path.c_str();
    try
    {
        // Print the image
        HINSTANCE result = ShellExecute(NULL, "print", image_path.c_str(), NULL, NULL, SW_HIDE);
        if ((int)result <= 32)
        {
            throw std::runtime_error("Failed to print");
        }

        request.reply(status_codes::OK, "Print job sent successfully");
    }
    catch (const std::exception& e)
    {
        request.reply(status_codes::InternalError, e.what());
    }
}

int main()
{
    uri_builder uri(U("http://localhost:3000"));
    auto addr = uri.to_uri().to_string();
    http_listener listener(addr);
    listener.support(methods::GET, handle_print_request);

    try
    {
        listener
            .open()
            .then([&listener]() { std::wcout << L"Starting server at: " << listener.uri().to_string() << std::endl; })
            .wait();

        std::string line;
        std::getline(std::cin, line);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
