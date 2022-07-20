#include "precompiled/pch.hpp"

#include "clock.hpp"

namespace RDE
{
uint32_t TypeID<Clock>::s_counter = 0;
uint32_t Clock::s_totalFrameTimings = 0;

uint32_t TypeID<Clock>::s_counter = 0;
uint32_t Clock::s_totalFrameTimings = 0;

Clock::Timer Clock::s_frameTimer;
Clock::Timer Clock::s_logTimer;
Clock::Timer Clock::s_timer;
Clock::Timer Clock::s_perSecondTimer;

bool Clock::s_isLogTimerRunning = false;

float Clock::s_compoundedFrameTiming = 0.0f;
float Clock::s_currentFps = 0.0f;

std::vector<float> Clock::s_perSecondDoTimes{};

Clock::Clock(const char *scopeName) : m_scopeName(scopeName) { start(s_timer); }

Clock::~Clock() {
    RDE_LOG_PROFILE("{0} finished in {1} ms", m_scopeName,
                    fmt::format("{:.{}f}", stop(s_timer), k_decimalPlaces));
}

[[nodiscard]] float Clock::stop(const Timer &timer) {
    auto stop = HRClock::now();
    std::chrono::duration<float, std::milli> ms = stop - timer;
    return ms.count();
}
} // namespace RDE
