#pragma once
#include "utilities/type_id.hpp"
#include <chrono>

namespace RDE
{

struct RDE::Logger;

class Clock
{
public:
  using Timer = std::chrono::time_point<std::chrono::high_resolution_clock>;
  using HRClock = std::chrono::high_resolution_clock;

  template <typename TFunc, typename... TArgs>
  static float profile(const char *funcName, TFunc &&func, TArgs &&...args)
  {
    static_assert(std::is_invocable_v<TFunc, TArgs...>,
                  "Function is not invocable!");

    start(s_timer);

    std::forward<TFunc>(func)(std::forward<TArgs>(args)...);

    float duration = stop(s_timer);
    RDE_LOG_PROFILE("{0} finished in {1} ms", funcName,
                    fmt::format("{:.{}f}", duration, k_decimalPlaces))

    return duration;
  }

  template <typename TFunc>
  static float profile(const char *funcName, TFunc &&func)
  {
    static_assert(std::is_invocable_v<TFunc>, "Function is not invocable!");

    start(s_timer);

    func();

    float duration = stop(s_timer);
    RDE_LOG_PROFILE("{0} finished in {1} ms", funcName,
                    fmt::format("{:.{}f}", duration, k_decimalPlaces))

    return duration;
  }

  // Returns delta time in seconds
  template <typename TFunc>
  static float deltaTime(TFunc &&func, bool log = false)
  {
    // if (log && !s_isLogTimerRunning) {
    //	start(s_logTimer);
    //	s_isLogTimerRunning = true;
    // }

    start(s_frameTimer);

    func();

    // float currentDt = stop(s_frameTimer) * k_milliToSeconds;
    // float logTimerDuration = stop(s_logTimer) * k_milliToSeconds;
    // s_currentFps = 1 / currentDt;
    //
    // s_compoundedFrameTiming += s_currentFps;
    // s_totalFrameTimings++;

    // if (logTimerDuration >= 1.0f) {
    //	float average = std::round(s_compoundedFrameTiming /
    //(float)s_totalFrameTimings); 	float averageDt = 1 / average; 	float
    // current = std::round(s_currentFps);
    //
    //	RDE_LOG_PROFILE("Average FPS: {0} ({1} ms), Current FPS: {2}
    //({3} ms), Number of frames passed: {4}", 		average,
    //		fmt::format("{:.{}f}", averageDt / k_milliToSeconds,
    // k_decimalPlaces), 		current,
    // fmt::format("{:.{}f}", currentDt / k_milliToSeconds, k_decimalPlaces),
    // s_totalFrameTimings);
    //
    //	start(s_logTimer);
    // }

    // for (float& timePassed : s_perSecondDoTimes) {
    //	timePassed += currentDt;
    // }

    // Recalculate dt for more accuracy
    float currentDt = stop(s_frameTimer) * k_milliToSeconds;

    return currentDt;
  }

  template <typename TCallable> static void perSecondDo(TCallable &&callable)
  {
    static_assert(std::is_invocable_v<TCallable>, "Function is not invocable!");

    uint32_t id = TypeID<Clock>::getID<TCallable>();
    if (id >= s_perSecondDoTimes.size()) {
      s_perSecondDoTimes.emplace_back(0.0f);
    }

    // If passed 1 second, call delegate
    if (s_perSecondDoTimes[id] >= 1.0f) {
      callable();
      s_perSecondDoTimes[id] = 0.0f;
    }
  }

  inline static void start(Timer &timer) { timer = HRClock::now(); }
  [[nodiscard]] static float stop(const Timer &timer);

  Clock(const char *scopeName = "");
  ~Clock();

private:
  static constexpr int k_decimalPlaces = 2;
  static constexpr float k_milliToSeconds = 0.001f;

  static Timer s_timer;

  // Frame timing member variables
  static Timer s_frameTimer;
  static Timer s_logTimer;
  static Timer s_perSecondTimer;
  static bool s_isLogTimerRunning;
  static uint32_t s_totalFrameTimings;
  static float s_compoundedFrameTiming;
  static float s_currentFps;
  static std::vector<float> s_perSecondDoTimes;

  std::string m_scopeName;
};
} // namespace RDE

#define RDE_PROFILE_SCOPE                                                      \
  RDE::Clock clock(fmt::format("{}()", __FUNCTION__).c_str());
#define RDE_LOG_PER_SECOND(callable) RDE::Clock::perSecondDo(callable);
