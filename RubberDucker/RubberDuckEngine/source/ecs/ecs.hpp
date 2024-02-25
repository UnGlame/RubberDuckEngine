#pragma once
#include "utilities/type_id.hpp"

#include <entt/entt.hpp>

namespace RDE {

class Engine;

class EntityComponentSystem
{
    using SystemType = std::unique_ptr<void, void (*)(void*)>;

public:
    EntityComponentSystem() = default;

    void init();

    template<typename TSystem>
    void registerSystem()
    {
        const uint32_t id = TypeID<EntityComponentSystem>::getId<TSystem>();

        if (id >= m_systems.size()) {
            // System does not exist in container, add it in.
            SystemType system{new TSystem{}, [](void* p) { delete static_cast<TSystem*>(p); }};
            m_systems.emplace_back(std::move(system));
            RDELOG_INFO("Adding new system {}, id {} into systems container", typeid(TSystem).name(), id);
        }
        // Setup delegate so that the system's update would be called on this->update()
        auto& delegate = m_updateDelegates.emplace_back();
        delegate.connect<&TSystem::update>(&getSystem<TSystem>());
    }

    void update(entt::registry& registry, float dt);
    void registerSystems();

private:
    template<typename TSystem>
    TSystem& getSystem()
    {
        uint32_t id = TypeID<EntityComponentSystem>::getId<TSystem>();
        RDE_ASSERT_0(id <= m_systems.size(), "Some system is not added into systems container!");

        return *static_cast<TSystem*>(m_systems[id].get());
    }

    std::vector<entt::delegate<void(entt::registry&, float)>> m_updateDelegates;
    std::vector<SystemType> m_systems;
};
} // namespace RDE
