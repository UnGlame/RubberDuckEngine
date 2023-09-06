#pragma once
#include <spdlog/fmt/ostr.h>
#include <spdlog/spdlog.h>

namespace RDE
{

struct Logger {
    static void init();
    static void test();

    static std::shared_ptr<spdlog::logger> s_logger;
    static std::shared_ptr<spdlog::logger> s_profileLogger;
};
} // namespace RDE

#define RDELOG_HELPER_DO_NOT_USE(...)                                                                                                      \
    fmt::format("{0}, {1}, {2}: {3}", RDE_LOCAL_FILENAME, __LINE__, __FUNCTION__, fmt::format(__VA_ARGS__))

#define RDELOG_INFO(...) RDE::Logger::s_logger->info(RDELOG_HELPER_DO_NOT_USE(__VA_ARGS__));
#define RDELOG_DEBUG(...) RDE::Logger::s_logger->debug(RDELOG_HELPER_DO_NOT_USE(__VA_ARGS__));
#define RDELOG_TRACE(...) RDE::Logger::s_logger->trace(RDELOG_HELPER_DO_NOT_USE(__VA_ARGS__));
#define RDELOG_WARN(...) RDE::Logger::s_logger->warn(RDELOG_HELPER_DO_NOT_USE(__VA_ARGS__));
#define RDELOG_ERROR(...) RDE::Logger::s_logger->error(RDELOG_HELPER_DO_NOT_USE(__VA_ARGS__));
#define RDELOG_CRITICAL(...) RDE::Logger::s_logger->critical(RDELOG_HELPER_DO_NOT_USE(__VA_ARGS__));

#define RDELOG_CLEAN_INFO(...) RDE::Logger::s_logger->info(fmt::format(__VA_ARGS__));
#define RDELOG_CLEAN_DEBUG(...) RDE::Logger::s_logger->debug(fmt::format(__VA_ARGS__));
#define RDELOG_CLEAN_TRACE(...) RDE::Logger::s_logger->trace(fmt::format(__VA_ARGS__));
#define RDELOG_CLEAN_WARN(...) RDE::Logger::s_logger->warn(fmt::format(__VA_ARGS__));
#define RDELOG_CLEAN_ERROR(...) RDE::Logger::s_logger->error(fmt::format(__VA_ARGS__));
#define RDELOG_CLEAN_CRITICAL(...) RDE::Logger::s_logger->critical(fmt::format(__VA_ARGS__));

#define RDELOG_PROFILE(...) RDE::Logger::s_profileLogger->debug(fmt::format(__VA_ARGS__));
