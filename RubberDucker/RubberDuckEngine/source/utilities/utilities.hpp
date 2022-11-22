#pragma once
#include <array>
#include <vector>

#include <glm/ext/scalar_constants.hpp>
#include <glm/gtc/epsilon.hpp>

#include "utilities/clock.hpp"
#include "utilities/file_parser.hpp"

namespace RDE
{
namespace Utilities
{
template <typename T> size_t arraysizeof(const std::vector<T>& vec) { return sizeof(T) * vec.size(); }

template <typename T, size_t Size> size_t arraysizeof(const std::array<T, Size>&) { return sizeof(T) * Size; }

template <typename T> bool floatEqual(T a, T b) { return glm::epsilonEqual(a, b, glm::epsilon<float>()); }

template <typename... T> constexpr bool always_false_v = false;

[[nodiscard]] inline uint64_t hash(uint32_t a, uint32_t b) { return a >= b ? a * a + a + b : a + b * b; }

} // namespace Utilities
} // namespace RDE
