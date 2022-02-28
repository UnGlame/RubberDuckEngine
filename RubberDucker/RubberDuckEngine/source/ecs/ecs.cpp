#include "precompiled/pch.hpp"
#include "ecs/ecs.hpp"

// Systems
#include "input/input_system.hpp"
#include "camera/camera_system.hpp"

namespace RDE {

    uint32_t TypeID<ECS>::s_counter = 0;

    ECS::ECS() :
        m_registry(std::make_unique<entt::registry>())
    {}

    void ECS::init()
    {
        createSystems();
        registerSystems();
    }

    void ECS::update(Engine* engine, float dt)
    {
        for (const auto& delegate : m_updateDelegates) {
            delegate(engine, dt);
        }
    }

    void ECS::createSystems()
    {
        createSystem<InputSystem>();
        createSystem<CameraSystem>();
    }

    void ECS::registerSystems()
    {
        registerSystem<InputSystem>();
        registerSystem<CameraSystem>();
    }
}