#include <pch.h>

#include "Logger.h"

std::shared_ptr<spdlog::sinks::stdout_color_sink_mt>    Logger::console_sink;
std::shared_ptr<spdlog::sinks::daily_file_sink_mt>      Logger::file_sink;
spdlog::logger*                                         Logger::logger;
std::vector<LogMessage>                                 Logger::messages;

int                                                     Logger::levelMessageCount[6] = {0,0,0,0,0,0};

void Logger::init() {
#ifdef DEBUG
    console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_level(spdlog::level::trace);
    console_sink->set_pattern("[%T:%e]%^ [%l] %v %$");
#endif

    file_sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>("logs/Toos_Log.log", 2, 30);
    file_sink->set_level(spdlog::level::trace);
    file_sink->set_pattern("[%T:%e] [%l] %v");

#ifdef DEBUG
    logger = new spdlog::logger("ToosLogger", {console_sink, file_sink});
#else
    logger = new spdlog::logger("ToosLogger", file_sink);
#endif

    //reserve space for a million log messages in memory
    messages.reserve(1000000);
}

void Logger::terminate() {
    logger->flush();
    delete logger;
}

