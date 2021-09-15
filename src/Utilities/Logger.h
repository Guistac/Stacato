#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/daily_file_sink.h>

#include "Timing.h"

class LogMessage {
public:
    LogMessage(std::string& msg, int l) : level(l){
        message = "[" + std::to_string(Timing::getTime_seconds()) + "s] " + std::move(msg);
    }
    inline const char* getMessage() { return message.c_str(); }
    inline bool isTrace() { return level == 0; }
    inline bool isDebug() { return level == 1; }
    inline bool isInfo() { return level == 2; }
    inline bool isWarn() { return level == 3; }
    inline bool isError() { return level == 4; }
    inline bool isCritical() { return level == 5; }
    std::string message;
    int level = -1;
};

class Logger{
public:

	static void init();
    static void terminate();

    static std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> console_sink;
    static std::shared_ptr<spdlog::sinks::daily_file_sink_mt> file_sink;
    static spdlog::logger* logger;

    static std::vector<LogMessage> messages;

    static int levelMessageCount[6];

    template<typename... Args>
    static bool trace(fmt::format_string<Args...> format, Args &&...args) {
        logger->trace(format, std::forward<Args>(args)...);
        std::string str = fmt::format(format, std::forward<Args>(args)...);
        messages.push_back(LogMessage(str, 0));
        levelMessageCount[0]++;
        return true;
    }

    template<typename... Args>
    static bool debug(fmt::format_string<Args...> format, Args &&...args) {
        logger->debug(format, std::forward<Args>(args)...);
        std::string str = fmt::format(format, std::forward<Args>(args)...);
        messages.push_back(LogMessage(str, 1));
        levelMessageCount[1]++;
        return true;
    }

    template<typename... Args>
    static bool info(fmt::format_string<Args...> format, Args &&...args) {
        logger->info(format, std::forward<Args>(args)...);
        std::string str = fmt::format(format, std::forward<Args>(args)...);
        messages.push_back(LogMessage(str, 2));
        levelMessageCount[2]++;
        return true;
    }

    template<typename... Args>
    static bool warn(fmt::format_string<Args...> format, Args &&...args) {
        logger->warn(format, std::forward<Args>(args)...);
        std::string str = fmt::format(format, std::forward<Args>(args)...);
        messages.push_back(LogMessage(str, 3));
        levelMessageCount[3]++;
        return false;
    }

    template<typename... Args>
    static bool error(fmt::format_string<Args...> format, Args &&...args) {
        logger->error(format, std::forward<Args>(args)...);
        std::string str = fmt::format(format, std::forward<Args>(args)...);
        messages.push_back(LogMessage(str, 4));
        levelMessageCount[4]++;
        return false;
    }

    template<typename... Args>
    static bool critical(fmt::format_string<Args...> format, Args &&...args) {
        logger->critical(format, std::forward<Args>(args)...);
        std::string str = fmt::format(format, std::forward<Args>(args)...);
        messages.push_back(LogMessage(str, 5));
        levelMessageCount[5]++;
        return false;
    }
};

