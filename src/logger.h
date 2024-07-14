#include "spdlog/sinks/rotating_file_sink.h"
void rotate_spdlog()
{
	// Create a file rotating logger with 5 MB size max and 3 rotated files
	auto max_size = 1048576 * 5;
	auto max_files = 3;
	auto logger = spdlog::rotating_logger_mt("ssi_logger", "logs/rotating.txt", max_size, max_files);
}