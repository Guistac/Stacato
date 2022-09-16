#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/daily_file_sink.h>

#include "Timing.h"

namespace Logger{

	class Message {
	public:
		Message(std::string& msg, int l) : level(l){ message = "[" + Timing::getTimeStringMillis() + "] " + std::move(msg);}
		inline const char* getString() { return message.c_str(); }
		inline bool isTrace() { return level == 0; }
		inline bool isDebug() { return level == 1; }
		inline bool isInfo() { return level == 2; }
		inline bool isWarn() { return level == 3; }
		inline bool isError() { return level == 4; }
		inline bool isCritical() { return level == 5; }
		std::string message;
		int level = -1;
	};

	struct Context{
		std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> console_sink;
		std::shared_ptr<spdlog::sinks::daily_file_sink_mt> file_sink;
		spdlog::logger* logger;
	};

	inline Context& getContext(){
		static Context loggerContext;
		return loggerContext;
	}

	inline std::vector<Message>& getMessages(){
		static std::vector<Message> messages = {};
		return messages;
	}

	inline void clearMessages(){
		getMessages().clear();
	}

	inline int* getLevelMessageCount(){
		static int levelMessageCount[6] = {0, 0, 0, 0, 0, 0};
		return levelMessageCount;
	}

	inline void init(){
		
		Context& context = getContext();
		
		context.console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
		context.console_sink->set_pattern("[%T:%e]%^ [%l] %v %$");
		context.console_sink->set_level(spdlog::level::trace);

		context.file_sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>("logs/StacatoLog.log", 2, 30);
		context.file_sink->set_pattern("[%T:%e] [%l] %v");
		context.file_sink->set_level(spdlog::level::trace);

		context.logger = new spdlog::logger("StacatoLog", {context.console_sink, context.file_sink});
		context.logger->set_level(spdlog::level::trace);

		//reserve space for a million log messages in memory
		getMessages().reserve(1000000);
	}

	inline void terminate(){
		Context& context = getContext();
		context.logger->flush();
		delete context.logger;
	}

	template<typename... Args>
	inline bool trace(fmt::format_string<Args...> format, Args &&...args) {
		getContext().logger->trace(format, std::forward<Args>(args)...);
		std::string str = fmt::format(format, std::forward<Args>(args)...);
		getMessages().push_back(Message(str, 0));
		getLevelMessageCount()[0]++;
		return true;
	}

	template<typename... Args>
	inline bool debug(fmt::format_string<Args...> format, Args &&...args) {
		getContext().logger->debug(format, std::forward<Args>(args)...);
		std::string str = fmt::format(format, std::forward<Args>(args)...);
		getMessages().push_back(Message(str, 1));
		getLevelMessageCount()[1]++;
		return true;
	}

	template<typename... Args>
	inline bool info(fmt::format_string<Args...> format, Args &&...args) {
		getContext().logger->info(format, std::forward<Args>(args)...);
		std::string str = fmt::format(format, std::forward<Args>(args)...);
		getMessages().push_back(Message(str, 2));
		getLevelMessageCount()[2]++;
		return true;
	}

	template<typename... Args>
	inline bool warn(fmt::format_string<Args...> format, Args &&...args) {
		getContext().logger->warn(format, std::forward<Args>(args)...);
		std::string str = fmt::format(format, std::forward<Args>(args)...);
		getMessages().push_back(Message(str, 3));
		getLevelMessageCount()[3]++;
		return false;
	}

	template<typename... Args>
	inline bool error(fmt::format_string<Args...> format, Args &&...args) {
		getContext().logger->error(format, std::forward<Args>(args)...);
		std::string str = fmt::format(format, std::forward<Args>(args)...);
		getMessages().push_back(Message(str, 4));
		getLevelMessageCount()[4]++;
		return false;
	}

	template<typename... Args>
	inline bool critical(fmt::format_string<Args...> format, Args &&...args) {
		getContext().logger->critical(format, std::forward<Args>(args)...);
		std::string str = fmt::format(format, std::forward<Args>(args)...);
		getMessages().push_back(Message(str, 5));
		getLevelMessageCount()[5]++;
		return false;
	}

}

