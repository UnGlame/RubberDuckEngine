#include "precompiled/pch.hpp"

#include "ecs/ecs.hpp"

#include "camera/camera_system.hpp"
#include "input/input_system.hpp"
#include "vulkan/systems/instance_update_system.hpp"

namespace RDE {

uint32_t TypeID<EntityComponentSystem>::s_counter = 0;

void EntityComponentSystem::init()
{
    registerSystems();
}

void EntityComponentSystem::update(entt::registry& registry, float dt)
{
    for (const auto& delegate : m_updateDelegates) {
        delegate(registry, dt);
    }
}

void EntityComponentSystem::registerSystems()
{
    registerSystem<InputSystem>();
    registerSystem<CameraSystem>();
    registerSystem<InstanceUpdateSystem>();
}
} // namespace RDE
