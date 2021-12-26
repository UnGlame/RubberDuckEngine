#include "pch.hpp"
#include "clock.hpp"

namespace RDE
{
	Clock::Time Clock::s_start;
	Clock::Time Clock::s_frameStart;
	Clock::Time Clock::s_timer;
	bool Clock::s_isTimerRunning = false;
	size_t Clock::s_timingIndex = 0;
	uint32_t Clock::s_totalTimings = 0;
	float Clock::s_compoundedTiming = 0.0f;
	std::array<float, Clock::c_numTimings> Clock::s_timings = { 0 };

	Clock::Clock(const char* scopeName) :
		m_scopeName(scopeName)
	{
		start(s_start);
	}
	
	Clock::~Clock()
	{
		RDE_LOG_PROFILE("{0} finished in {1} ms", m_scopeName, fmt::format("{:.{}f}", stop(s_start), c_decimalPlaces));
	}

	[[nodiscard]]
	float Clock::stop(const Time& start)
	{
		auto stop = HRClock::now();
		std::chrono::duration<float, std::milli> ms = stop - start;
		return ms.count();
	}
}