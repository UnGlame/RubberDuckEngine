#pragma once
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

namespace RDE
{
	struct Logger
	{
		static void init();
		static void test();

		static std::shared_ptr<spdlog::logger> s_logger;
	};
}


#define __LOCAL_FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#define RDE_LOG_HELPER_DO_NOT_USE(...) fmt::format("{0}, {1}, {2}: {3}", __LOCAL_FILENAME__, __LINE__, __FUNCTION__, fmt::format(__VA_ARGS__))

#define RDE_LOG_INFO(...) RDE::Logger::s_logger->info(RDE_LOG_HELPER_DO_NOT_USE(__VA_ARGS__));
#define RDE_LOG_DEBUG(...) RDE::Logger::s_logger->debug(RDE_LOG_HELPER_DO_NOT_USE(__VA_ARGS__));
#define RDE_LOG_TRACE(...) RDE::Logger::s_logger->trace(RDE_LOG_HELPER_DO_NOT_USE(__VA_ARGS__));
#define RDE_LOG_WARN(...) RDE::Logger::s_logger->warn(RDE_LOG_HELPER_DO_NOT_USE(__VA_ARGS__));
#define RDE_LOG_ERROR(...) RDE::Logger::s_logger->error(RDE_LOG_HELPER_DO_NOT_USE(__VA_ARGS__));
#define RDE_LOG_CRITICAL(...) RDE::Logger::s_logger->critical(RDE_LOG_HELPER_DO_NOT_USE(__VA_ARGS__));