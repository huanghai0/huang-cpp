#ifndef LOGGER_H
#define LOGGER_H

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <memory>
#include <string>

class Logger
{
private:
    static std::shared_ptr<spdlog::logger> logger;

public:
    static bool initialize(const std::string &logFile = "app.log");
    static std::shared_ptr<spdlog::logger> getLogger();

    // 便捷日志方法
    template <typename... Args>
    static void trace(const char *fmt, const Args &...args)
    {
        if (logger)
            logger->trace(fmt, args...);
    }

    template <typename... Args>
    static void debug(const char *fmt, const Args &...args)
    {
        if (logger)
            logger->debug(fmt, args...);
    }

    template <typename... Args>
    static void info(const char *fmt, const Args &...args)
    {
        if (logger)
            logger->info(fmt, args...);
    }

    template <typename... Args>
    static void warn(const char *fmt, const Args &...args)
    {
        if (logger)
            logger->warn(fmt, args...);
    }

    template <typename... Args>
    static void error(const char *fmt, const Args &...args)
    {
        if (logger)
            logger->error(fmt, args...);
    }

    template <typename... Args>
    static void critical(const char *fmt, const Args &...args)
    {
        if (logger)
            logger->critical(fmt, args...);
    }
};

#endif // LOGGER_H
