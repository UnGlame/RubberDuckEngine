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
			float fps = 1000.0f / frameDuration;

			s_compoundedTiming += fps;
			s_totalTimings++;

			s_timings[s_timingIndex] = fps;
			s_timingIndex = (s_timingIndex + 1) % c_numTimings;

			if (timerDuration >= 1000.0f) {
				float average = 0.0f;

				for (float timing : s_timings) {
					average += timing;
				}
				average /= c_numTimings;

				RDE_LOG_PROFILE("Current FPS: {0}, Overall Average FPS: {1}, Fixed Average FPS: {2}",
					std::round(fps), std::round(s_compoundedTiming / (float)s_totalTimings), std::round(average));

				start(s_timer);
			}
			return frameDuration;
		}

		Clock(const char* scopeName = "");
		~Clock();

	private:
		static constexpr int c_decimalPlaces = 2;
		static constexpr size_t c_numTimings = 4096;

		static Time s_start;
		static Time s_frameStart;

		static Time s_timer;
		static bool s_isTimerRunning;
		static size_t s_timingIndex;
		static uint32_t s_totalTimings;
		static float s_compoundedTiming;
		static std::array<float, c_numTimings> s_timings;

		std::string m_scopeName;

		__forceinline static void start(Time& start) { start = HRClock::now(); }
		[[nodiscard]] static float stop(const Time& start);
	};
}

#define RDE_PROFILE_SCOPE RDE::Clock clock(fmt::format("{}()", __FUNCTION__).c_str());