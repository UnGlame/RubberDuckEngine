#include "precompiled/pch.hpp"

#include "ecs/components/component_list.hpp"

#include <rttr/registration.h>

RTTR_REGISTRATION
{
    // Non-components
    rttr::registration::class_<glm::vec3>("glm::vec3")
        .property("x", &glm::vec3::x)
        .property("y", &glm::vec3::y)
        .property("z", &glm::vec3::z);

    rttr::registration::class_<glm::quat>("glm::quat")
        .property("w", &glm::quat::w)
        .property("x", &glm::quat::x)
        .property("y", &glm::quat::y)
        .property("z", &glm::quat::z);

    // Components
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
