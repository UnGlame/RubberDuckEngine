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
		static float profile(const char* funcName, TFunc&& func, TArgs&&... args) {
			RDE_LOG_DEBUG("Profiling {}...", funcName);
			start();

			std::forward<TFunc>(func)(std::forward<TArgs>(args)...);

			float duration = stop();
			RDE_LOG_DEBUG("...{0} finished in {1} ms.\n", funcName, fmt::format("{:.{}f}", duration, 1))

			return duration;
		}

		template <typename TFunc>
		static float profile(const char* funcName, TFunc&& func) {
			RDE_LOG_DEBUG("Profiling {}...", funcName);
			start();

			func();

			float duration = stop();
			RDE_LOG_DEBUG("...{0} finished in {1} ms.\n", funcName, fmt::format("{:.{}f}", duration, 1))

			return duration;
		}
		
		Clock(const char* scopeName = "");
		~Clock();

	private:
		static Time s_start;
		std::string m_scopeName;

		__forceinline static void start() { s_start = HRClock::now(); }
		[[nodiscard]] static float stop();
	};
}