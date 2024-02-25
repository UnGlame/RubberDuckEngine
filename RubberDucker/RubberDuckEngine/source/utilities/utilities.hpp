#pragma once
#include "utilities/clock.hpp"
#include "utilities/file_parser.hpp"

#include <entt/entt.hpp>
#include <glm/ext/scalar_constants.hpp>
#include <glm/gtc/epsilon.hpp>

#include <array>
#include <vector>

namespace RDE {
namespace Utilities {

template<typename T>
size_t arraysizeof(const std::vector<T>& vec)
{
    return sizeof(T) * vec.size();
}

template<typename T, size_t Size>
size_t arraysizeof(const std::array<T, Size>&)
{
    return sizeof(T) * Size;
}

template<typename T>
bool floatEqual(T a, T b)
{
    return glm::epsilonEqual(a, b, glm::epsilon<float>());
}

template<typename... T>
constexpr bool always_false_v = false;

[[nodiscard]] inline uint64_t hash(uint32_t a, uint32_t b)
{
    return a >= b ? a * a + a + b : a + b * b;
}

std::vector<entt::type_info> getComponentTypeInfos(entt::registry& registry, entt::entity entity);

std::unordered_map<entt::entity, std::vector<entt::type_info>> getAllEntityComponentTypeInfos(entt::registry& registry);

} // namespace Utilities
} // namespace RDE
