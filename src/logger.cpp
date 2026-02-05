#include "logger.h"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <iostream>

// 静态成员初始化
std::shared_ptr<spdlog::logger> Logger::logger = nullptr;

bool Logger::initialize(const std::string &logFile)
{
    try
    {
        // 创建控制台和文件输出
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFile, true);

        // 设置日志格式
        console_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%t] [%^%l%$] [%s:%#] %v");
        file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%t] [%l] [%s:%#] %v");

        // 创建组合logger
        logger = std::make_shared<spdlog::logger>("main", spdlog::sinks_init_list{console_sink, file_sink});

        // 设置日志级别
#ifdef NDEBUG
        logger->set_level(spdlog::level::info);
#else
        logger->set_level(spdlog::level::debug);
#endif

        // 设置刷新策略
        logger->flush_on(spdlog::level::warn);

        spdlog::register_logger(logger);
        spdlog::set_default_logger(logger);

        logger->info("Logger initialized successfully");
        return true;
    }
    catch (const spdlog::spdlog_ex &ex)
    {
        std::cerr << "Log initialization failed: " << ex.what() << std::endl;
        return false;
    }
}

std::shared_ptr<spdlog::logger> Logger::getLogger()
{
    return logger;
}
