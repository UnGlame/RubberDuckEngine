#include "ecs/ecs.hpp"
#include "precompiled/pch.hpp"

// Systems
#include "camera/camera_system.hpp"
<<<<<<< HEAD
#include "editor/hierarchy/hierarchy_system.hpp"
=======
#include "input/input_system.hpp"
>>>>>>> main
#include "vulkan/systems/instance_update_system.hpp"

namespace RDE
{

uint32_t TypeID<ECS>::s_counter = 0;

ECS::ECS() : m_registry(std::make_unique<entt::registry>()) {}

void ECS::init()
{
    createSystems();
    registerSystems();
}

void ECS::update(float dt)
{
    for (const auto& delegate : m_updateDelegates) {
        delegate(*m_registry, dt);
    }
}

void ECS::createSystems()
{
    createSystem<InputSystem>();
    createSystem<CameraSystem>();
    createSystem<InstanceUpdateSystem>();
}

<<<<<<< HEAD
    void ECS::createSystems()
    {
        createSystem<InputSystem>();
        createSystem<CameraSystem>();
        createSystem<InstanceUpdateSystem>();
        createSystem<HierarchySystem>();
    }

    void ECS::registerSystems()
    {
        registerSystem<InputSystem>();
        registerSystem<CameraSystem>();
        registerSystem<InstanceUpdateSystem>();
        registerSystem<HierarchySystem>();
    }
}
=======
void ECS::registerSystems()
{
    registerSystem<InputSystem>();
    registerSystem<CameraSystem>();
    registerSystem<InstanceUpdateSystem>();
}
} // namespace RDE
>>>>>>> main
