#pragma once
#include <cassert>
#include <memory>

#include "logger/logger.hpp"

#define RDE_ENABLE_ASSERT_LVL_1

#define RDE_LOCAL_FILENAME                                                     \
    (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)

#ifdef RDE_DEBUG
#if defined(RDE_ENABLE_ASSERT_LVL_0)
#define RDE_ASSERT_0(x, ...)                                                   \
    {                                                                          \
        if (!(x)) {                                                            \
            RDE_LOG_CRITICAL("Assertion 0 Failed: {}",                         \
                             fmt::format(__VA_ARGS__));                        \
            assert(x);                                                         \
        }                                                                      \
    } // Asserts called mostly once (eg loading assets)
#define RDE_ASSERT_1(x, ...)
#define RDE_ASSERT_2(x, ...)
#elif defined(RDE_ENABLE_ASSERT_LVL_1)
#define RDE_ASSERT_0(x, ...)                                                   \
    {                                                                          \
        if (!(x)) {                                                            \
            RDE_LOG_CRITICAL("Assertion 0 Failed: {}",                         \
                             fmt::format(__VA_ARGS__));                        \
            assert(x);                                                         \
        }                                                                      \
    } // Asserts called mostly once (eg loading assets)
#define RDE_ASSERT_1(x, ...)                                                   \
    {                                                                          \
        if (!(x)) {                                                            \
            RDE_LOG_CRITICAL("Assertion 1 Failed: {}",                         \
                             fmt::format(__VA_ARGS__));                        \
            assert(x);                                                         \
        }                                                                      \
    } // Asserts called every frame and will cause crashes if assert
#define RDE_ASSERT_2(x, ...)
#elif defined(RDE_ENABLE_ASSERT_LVL_2)
#define RDE_ASSERT_0(x, ...)                                                   \
    {                                                                          \
        if (!(x)) {                                                            \
            RDE_LOG_CRITICAL("Assertion 0 Failed: {}",                         \
                             fmt::format(__VA_ARGS__));                        \
            assert(x);                                                         \
        }                                                                      \
    } // Asserts called mostly once (e.g. loading assets)
#define RDE_ASSERT_1(x, ...)                                                   \
    {                                                                          \
        if (!(x)) {                                                            \
            RDE_LOG_CRITICAL("Assertion 1 Failed: {}",                         \
                             fmt::format(__VA_ARGS__));                        \
            assert(x);                                                         \
        }                                                                      \
    } // Asserts called every frame and will cause crashes if assert
#define RDE_ASSERT_2(x, ...)                                                   \
    {                                                                          \
        if (!(x)) {                                                            \
            RDE_LOG_CRITICAL("Assertion 2 Failed: {}",                         \
                             fmt::format(__VA_ARGS__));                        \
            assert(x);                                                         \
        }                                                                      \
    } // Asserts called every frame and will cause undefined behaviour if
      // asserted
#endif
#else // NDEBUG
      // Strip away all asserts
#define RDE_ASSERT_0(x, ...) (x)
#define RDE_ASSERT_1(x, ...) (x)
#define RDE_ASSERT_2(x, ...) (x)
#endif
