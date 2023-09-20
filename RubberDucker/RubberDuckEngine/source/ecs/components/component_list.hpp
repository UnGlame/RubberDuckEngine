#pragma once
#include "model_component.hpp"
#include "transform_component.hpp"

#include <entt/entt.hpp>

namespace RDE
{
using ComponentList = entt::type_list<TransformComponent, ModelComponent, float, uint32_t, std::unordered_map<int, int>>;
constexpr ComponentList componentList_v{};

template <typename T, typename... Types>
inline void appendTypeNames(std::vector<std::string_view>& nameList, entt::type_list<T, Types...>) {
    nameList.emplace_back(entt::type_name<T>::value());
    appendTypeNames(nameList, entt::type_list<Types...>());
}

template <typename T>
inline void appendTypeNames(std::vector<std::string_view>& nameList, entt::type_list<T>) {
    nameList.emplace_back(entt::type_name<T>::value());
}
} // namespace RDE
