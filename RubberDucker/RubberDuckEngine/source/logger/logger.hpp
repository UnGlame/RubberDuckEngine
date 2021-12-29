#pragma once
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

namespace RDE {

	struct Logger
	{
		static void init();
		static void test();

		static std::shared_ptr<spdlog::logger> s_logger;
		static std::shared_ptr<spdlog::logger> s_profileLogger;
	};
}


#define RDE_LOG_HELPER_DO_NOT_USE(...)	fmt::format("{0}, {1}, {2}: {3}", RDE_LOCAL_FILENAME, __LINE__, __FUNCTION__, fmt::format(__VA_ARGS__))

#define RDE_LOG_INFO(...)			RDE::Logger::s_logger->info(RDE_LOG_HELPER_DO_NOT_USE(__VA_ARGS__));
#define RDE_LOG_DEBUG(...)			RDE::Logger::s_logger->debug(RDE_LOG_HELPER_DO_NOT_USE(__VA_ARGS__));
#define RDE_LOG_TRACE(...)			RDE::Logger::s_logger->trace(RDE_LOG_HELPER_DO_NOT_USE(__VA_ARGS__));
#define RDE_LOG_WARN(...)			RDE::Logger::s_logger->warn(RDE_LOG_HELPER_DO_NOT_USE(__VA_ARGS__));
#define RDE_LOG_ERROR(...)			RDE::Logger::s_logger->error(RDE_LOG_HELPER_DO_NOT_USE(__VA_ARGS__));
#define RDE_LOG_CRITICAL(...)		RDE::Logger::s_logger->critical(RDE_LOG_HELPER_DO_NOT_USE(__VA_ARGS__));

#define RDE_LOG_CLEAN_INFO(...)		RDE::Logger::s_logger->info(fmt::format(__VA_ARGS__));
#define RDE_LOG_CLEAN_DEBUG(...)	RDE::Logger::s_logger->debug(fmt::format(__VA_ARGS__));
#define RDE_LOG_CLEAN_TRACE(...)	RDE::Logger::s_logger->trace(fmt::format(__VA_ARGS__));
#define RDE_LOG_CLEAN_WARN(...)		RDE::Logger::s_logger->warn(fmt::format(__VA_ARGS__));
#define RDE_LOG_CLEAN_ERROR(...)	RDE::Logger::s_logger->error(fmt::format(__VA_ARGS__));
#define RDE_LOG_CLEAN_CRITICAL(...) RDE::Logger::s_logger->critical(fmt::format(__VA_ARGS__));

#define RDE_LOG_PROFILE(...)		RDE::Logger::s_profileLogger->info(fmt::format(__VA_ARGS__));