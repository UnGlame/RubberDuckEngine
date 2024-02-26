#pragma once
#include "entity_component.hpp"
#include "mesh_component.hpp"
#include "transform_component.hpp"

#include <rttr/registration.h>

namespace RDE {
using ComponentList = entt::type_list<EntityComponent, TransformComponent, MeshComponent>;
constexpr ComponentList k_componentList{};

template<typename T, typename... Types>
inline void appendTypeNames(std::vector<std::string_view>& nameList, entt::type_list<T, Types...>)
{
    nameList.emplace_back(entt::type_name<T>::value());
    appendTypeNames(nameList, entt::type_list<Types...>());
}

template<typename T>
inline void appendTypeNames(std::vector<std::string_view>& nameList, entt::type_list<T>)
{
    nameList.emplace_back(entt::type_name<T>::value());
}
} // namespace RDE

RTTR_REGISTRATION
{
    rttr::registration::class_<RDE::EntityComponent>("EntityComponent")
        .constructor<>()
        .property("name", &RDE::EntityComponent::name);

    rttr::registration::class_<RDE::TransformComponent>("TransformComponent")
        .constructor<>()
        .property("rotate", &RDE::TransformComponent::rotate)
        .property("scale", &RDE::TransformComponent::scale)
        .property("translate", &RDE::TransformComponent::translate);

    rttr::registration::class_<RDE::MeshComponent>("MeshComponent")
        .constructor<>()
        .property("modelGuid", &RDE::MeshComponent::modelGuid)
        .property("textureGuid", &RDE::MeshComponent::textureGuid);
}
