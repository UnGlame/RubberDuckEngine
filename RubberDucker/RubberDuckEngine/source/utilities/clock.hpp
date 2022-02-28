#pragma once
#include <chrono>

namespace RDE {

	struct RDE::Logger;

	class Clock
	{
	public:
		using Timer = std::chrono::time_point<std::chrono::high_resolution_clock>;
		using HRClock = std::chrono::high_resolution_clock;

		template <typename TFunc, typename... TArgs>
		static float profile(const char* funcName, TFunc&& func, TArgs&&... args)
		{
			static_assert(std::is_invocable_v<TFunc, TArgs...>, "Function is not invocable!");

			start(s_timer);

			std::forward<TFunc>(func)(std::forward<TArgs>(args)...);

			float duration = stop(s_timer);
			RDE_LOG_PROFILE("{0} finished in {1} ms", funcName, fmt::format("{:.{}f}", duration, s_decimalPlaces))

			return duration;
		}

		template <typename TFunc>
		static float profile(const char* funcName, TFunc&& func)
		{
			static_assert(std::is_invocable_v<TFunc>, "Function is not invocable!");

			start(s_timer);

			func();

			float duration = stop(s_timer);
			RDE_LOG_PROFILE("{0} finished in {1} ms", funcName, fmt::format("{:.{}f}", duration, s_decimalPlaces))

			return duration;
		}

		// Returns delta time in seconds
		template <typename TFunc>
		static float deltaTime(TFunc&& func, bool log = false)
		{
			if (log && !s_isLogTimerRunning) {
				start(s_logTimer);
				s_isLogTimerRunning = true;
			}

			start(s_frameTimer);

			func();

			float dt = stop(s_frameTimer) * k_milliToSeconds;
			float logTimerDuration = stop(s_logTimer) * k_milliToSeconds;
			s_currentFps = 1 / dt;

			s_compoundedFrameTiming += s_currentFps;
			s_totalFrameTimings++;

			if (logTimerDuration >= 1.0f) {
				RDE_LOG_PROFILE("Current FPS: {0}, Overall average FPS: {1}, Number of frames passed: {2}",
					std::round(s_currentFps), std::round(s_compoundedFrameTiming / (float)s_totalFrameTimings), s_totalFrameTimings);

				start(s_logTimer);
			}
			return dt;
		}

		inline static void start(Timer& timer) { timer = HRClock::now(); }
		[[nodiscard]] static float stop(const Timer& timer);

		Clock(const char* scopeName = "");
		~Clock();

	private:
		static constexpr int k_decimalPlaces = 2;
		static constexpr float k_milliToSeconds = 0.001f;

		static Timer s_timer;

		// Frame timing member variables
		static Timer s_frameTimer;
		static Timer s_logTimer;
		static bool s_isLogTimerRunning;
		static uint32_t s_totalFrameTimings;
		static float s_compoundedFrameTiming;
		static float s_currentFps;

		std::string m_scopeName;
	};
}

#define RDE_PROFILE_SCOPE RDE::Clock clock(fmt::format("{}()", __FUNCTION__).c_str());