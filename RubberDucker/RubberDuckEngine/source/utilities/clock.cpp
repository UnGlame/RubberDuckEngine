#include "pch.hpp"
#include "clock.hpp"

namespace RDE
{
	Clock::Time Clock::s_start;

	Clock::Clock(const char* scopeName) :
		m_scopeName(scopeName)
	{
		RDE_LOG_DEBUG("Profiling {}...", m_scopeName);
		start();
	}
	
	Clock::~Clock()
	{
		RDE_LOG_DEBUG("...{0} finished in {1} ms.\n", m_scopeName, fmt::format("{:.{}f}", stop(), 1));
	}

	[[nodiscard]]
	float Clock::stop()
	{
		auto stop = HRClock::now();
		std::chrono::duration<float, std::milli> ms = stop - s_start;
		return ms.count();
	}
}