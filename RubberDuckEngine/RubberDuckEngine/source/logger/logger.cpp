#include "pch.hpp"
#include <spdlog/sinks/stdout_color_sinks.h>

namespace RDE
{
	std::shared_ptr<spdlog::logger> Logger::s_logger = nullptr;

	void Logger::init()
	{
		spdlog::set_pattern("%^[%T][%n %l] %v%$");
		s_logger = spdlog::stdout_color_st("RDE");
		spdlog::set_level(spdlog::level::trace);
	}

	void Logger::test()
	{
		const char buffer[] = "buffer string";
		const auto bufferLen = strlen(buffer);
		char* heapString = new char[bufferLen + 1];
		strncpy(heapString, buffer, bufferLen + 1);
		std::ostringstream ss;
		ss << "stringstream";

		RDE_LOG_INFO(std::string("C++ string"));
		//RDE_LOG_DEBUG("c string");
		RDE_LOG_TRACE(buffer);
		RDE_LOG_WARN(ss.str());
		RDE_LOG_ERROR(ss.str().c_str());
		RDE_LOG_CRITICAL(heapString);

		delete[] heapString;
	}
}