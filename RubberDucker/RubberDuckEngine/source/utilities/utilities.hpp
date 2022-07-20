#pragma once
#include <array>
#include <vector>

#include <glm/ext/scalar_constants.hpp>
#include <glm/gtc/epsilon.hpp>

#include "utilities/clock.hpp"
#include "utilities/file_parser.hpp"

namespace RDE {
namespace Utilities {
template <typename T> size_t arraysizeof(const std::vector<T> &vec) {
    return sizeof(T) * vec.size();
}

template <typename T, size_t Size>
size_t arraysizeof(const std::array<T, Size> &) {
    return sizeof(T) * Size;
}

template <typename T> bool floatEqual(T a, T b) {
    return glm::epsilonEqual(a, b, glm::epsilon<float>());
}

template <typename... T> constexpr bool always_false_v = false;
} // namespace Utilities
} // namespace RDE
