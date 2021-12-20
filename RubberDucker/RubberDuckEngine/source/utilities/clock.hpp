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
			//RDE_LOG_PROFILE("Profiling {}", funcName);
			start();

			std::forward<TFunc>(func)(std::forward<TArgs>(args)...);

			float duration = stop();
			RDE_LOG_PROFILE("{0} finished in {1} ms", funcName, fmt::format("{:.{}f}", duration, s_decimalPlaces))

			return duration;
		}

		template <typename TFunc>
		static float profile(const char* funcName, TFunc&& func) {
			//RDE_LOG_DEBUG("Profiling {}", funcName);
			start();

			func();

			float duration = stop();
			RDE_LOG_PROFILE("{0} finished in {1} ms", funcName, fmt::format("{:.{}f}", duration, s_decimalPlaces))

			return duration;
		}
		
		Clock(const char* scopeName = "");
		~Clock();

	private:
		static constexpr int s_decimalPlaces = 2;

		static Time s_start;
		std::string m_scopeName;

		__forceinline static void start() { s_start = HRClock::now(); }
		[[nodiscard]] static float stop();
	};
}

#define RDE_PROFILE_SCOPE RDE::Clock clock(fmt::format("{}()", __FUNCTION__).c_str());