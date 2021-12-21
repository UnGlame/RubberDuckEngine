#include "pch.hpp"
#include "clock.hpp"

namespace RDE
{
	Clock::Time Clock::s_start;
	Clock::Time Clock::s_frameStart;
	Clock::Time Clock::s_timer;
	bool Clock::s_isTimerRunning = false;

	Clock::Clock(const char* scopeName) :
		m_scopeName(scopeName)
	{
		start(s_start);
	}
	
	Clock::~Clock()
	{
		RDE_LOG_PROFILE("{0} finished in {1} ms", m_scopeName, fmt::format("{:.{}f}", stop(s_start), s_decimalPlaces));
	}

	[[nodiscard]]
	float Clock::stop(const Time& start)
	{
		auto stop = HRClock::now();
		std::chrono::duration<float, std::milli> ms = stop - start;
		return ms.count();
	}
}