#pragma once
#include "model_component.hpp"
#include "transform_component.hpp"
#include "utilities/type_list.hpp"

namespace RDE {

template <typename... Types> struct ComponentTypeList : TypeList<Types...> {};

template <typename... Types>
constexpr ComponentTypeList<Types...> componentTypes_v{};

using ComponentList = ComponentTypeList<TransformComponent, ModelComponent>;
} // namespace RDE
