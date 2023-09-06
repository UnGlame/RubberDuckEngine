#include "precompiled/pch.hpp"
#include <spdlog/sinks/stdout_color_sinks.h>

namespace RDE
{

std::shared_ptr<spdlog::logger> Logger::s_logger = nullptr;
std::shared_ptr<spdlog::logger> Logger::s_profileLogger = nullptr;

void Logger::init()
{
    s_logger = spdlog::stdout_color_st("main");
    s_logger->set_pattern("[%T][%^%l%$] %v");
    s_logger->set_level(spdlog::level::trace);

    s_profileLogger = spdlog::stdout_color_st("profile");
    s_profileLogger->set_pattern("[%T][%^%n%$] %v");
    s_profileLogger->set_level(spdlog::level::trace);
}

void Logger::test()
{
    const char buffer[] = "buffer string";
    const auto bufferLen = strlen(buffer);
    char* heapString = new char[bufferLen + 1];
    strncpy(heapString, buffer, bufferLen + 1);
    std::ostringstream ss;
    ss << "stringstream";

    RDELOG_INFO(std::string("C++ string"));
    // RDELOG_DEBUG("c string");
    RDELOG_TRACE(buffer);
    RDELOG_WARN(ss.str());
    RDELOG_ERROR(ss.str().c_str());
    RDELOG_CRITICAL(heapString);

    delete[] heapString;
}
} // namespace RDE
