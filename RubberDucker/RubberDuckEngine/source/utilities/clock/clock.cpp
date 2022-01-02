#include "precompiled/pch.hpp"
#include "clock.hpp"

namespace RDE {

	Clock::Timer Clock::s_timer;
	Clock::Timer Clock::s_frameTimer;
	Clock::Timer Clock::s_logTimer;
	bool Clock::s_isLogTimerRunning = false;
	uint32_t Clock::s_totalFrameTimings = 0;
	float Clock::s_compoundedFrameTiming = 0.0f;

	Clock::Clock(const char* scopeName) :
		m_scopeName(scopeName)
	{
		start(s_timer);
	}
	
	Clock::~Clock()
	{
		RDE_LOG_PROFILE("{0} finished in {1} ms", m_scopeName, fmt::format("{:.{}f}", stop(s_timer), k_decimalPlaces));
	}

	[[nodiscard]]
	float Clock::stop(const Timer& timer)
	{
		auto stop = HRClock::now();
		std::chrono::duration<float, std::milli> ms = stop - timer;
		return ms.count();
	}
}