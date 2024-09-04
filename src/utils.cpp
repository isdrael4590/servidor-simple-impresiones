#include "pch.h"
#include "utils.h"

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
	spdlog::info("imageWidth: {}, imageHeight: {}", imageWidth, imageHeight);

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

void convert_pdf_to_images(const std::wstring& pdfPath, std::vector<std::string>& imagePaths)
{
    std::wcout << L"Converting PDF to images..." << std::endl;

    std::wstring outputPattern = L"output_page_%03d.png";
    std::wstring command = L"gswin64c -dBATCH -dNOPAUSE -sDEVICE=png16m -r300 -sOutputFile=" + outputPattern + L" " + pdfPath;

    _wsystem(command.c_str());

    std::filesystem::path current_path = std::filesystem::current_path();

    for (const auto& entry : std::filesystem::directory_iterator(current_path)) {
        if (entry.is_regular_file() && entry.path().extension() == ".png") {
            imagePaths.push_back(std::filesystem::relative(entry.path(), current_path).string());
            std::cout << std::filesystem::relative(entry.path(), current_path).string() << std::endl;
        }
    }
    spdlog::info("Convertidas {} images del PDF", imagePaths.size());
}

void delete_old_folders(const std::wstring& baseDir, int days)
{
    std::wcout << L"Deleting folders older than " << days << " days..." << std::endl;

    auto now = std::filesystem::file_time_type::clock::now();
    auto ageLimit = now - std::chrono::hours(days * 24);

    for (const auto& entry : std::filesystem::directory_iterator(baseDir))
    {
        if (std::filesystem::is_directory(entry))
        {
            auto ftime = std::filesystem::last_write_time(entry);
            if (ftime < ageLimit)
            {
                std::filesystem::remove_all(entry);
                std::wcout << L"Deleted folder: " << entry.path().wstring() << std::endl;
            }
        }
    }
}

std::wstring get_current_time_folder()
{
    auto now = std::time(nullptr);
    std::tm tm;
    localtime_s(&tm, &now);

    wchar_t buffer[100];
    wcsftime(buffer, 100, L"%Y_%m_%d_%H_%M_%S", &tm);

    return buffer;
}

