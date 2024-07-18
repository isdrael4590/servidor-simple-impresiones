#include "spdlog/spdlog.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"

class Logger {
public:
    static void init() {
        try {
            auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>("logs/logs.txt", 1048576 * 5, 3);
            file_sink->set_level(spdlog::level::info);

            spdlog::sinks_init_list sinks{ console_sink, file_sink };
            auto logger = std::make_shared<spdlog::logger>("multi_sink", begin(sinks), end(sinks));
            spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
            spdlog::set_level(spdlog::level::info);
            spdlog::set_default_logger(logger);
            spdlog::info("Logger inicializado");
        }
        catch (const spdlog::spdlog_ex& ex) {
            std::cerr << "Log initialization failed: " << ex.what() << std::endl;
        }
    }
};