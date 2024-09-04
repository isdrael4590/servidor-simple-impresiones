#pragma once

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

bool printImageToPrinter(const std::string& printerName, const std::string& imagePath);
void convert_pdf_to_images(const std::wstring& pdfPath, std::vector<std::string>& imagePaths);
void delete_old_folders(const std::wstring& baseDir, int days);
std::wstring get_current_time_folder();