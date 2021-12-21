#pragma once
#include <chrono>

namespace RDE
{
	struct RDE::Logger;

	class Clock
	{
	public:
		using Time = std::chrono::time_point<std::chrono::high_resolution_clock>;
		using HRClock = std::chrono::high_resolution_clock;

		template <typename TFunc, typename... TArgs>
		static float profile(const char* funcName, TFunc&& func, TArgs&&... args)
		{
			start(s_start);

			std::forward<TFunc>(func)(std::forward<TArgs>(args)...);

			float duration = stop(s_start);
			RDE_LOG_PROFILE("{0} finished in {1} ms", funcName, fmt::format("{:.{}f}", duration, s_decimalPlaces))

			return duration;
		}

		template <typename TFunc>
		static float profile(const char* funcName, TFunc&& func)
		{
			start(s_start);

			func();

			float duration = stop(s_start);
			RDE_LOG_PROFILE("{0} finished in {1} ms", funcName, fmt::format("{:.{}f}", duration, s_decimalPlaces))

			return duration;
		}

		template <typename TFunc>
		static float framesPerSecond(TFunc&& func)
		{
			if (!s_isTimerRunning) {
				start(s_timer);
				s_isTimerRunning = true;
			}

			start(s_frameStart);

			func();

			float frameDuration = stop(s_frameStart);
			float timerDuration = stop(s_timer);
			
			if (timerDuration >= 1000.0f) {
				start(s_timer);
				RDE_LOG_PROFILE("Current FPS: {}", std::round(1000 / frameDuration))
			}
			return frameDuration;
		}

		Clock(const char* scopeName = "");
		~Clock();

	private:
		static constexpr int s_decimalPlaces = 2;

		static Time s_start;
		static Time s_frameStart;

		static Time s_timer;
		static bool s_isTimerRunning;

		std::string m_scopeName;

		__forceinline static void start(Time& start) { start = HRClock::now(); }
		[[nodiscard]] static float stop(const Time& start);
	};
}

#define RDE_PROFILE_SCOPE RDE::Clock clock(fmt::format("{}()", __FUNCTION__).c_str());