#pragma once
#include "entt/entt.hpp"
#include "utilities/type_id.hpp"

namespace RDE
{

class Engine;

class ECS
{
    using SystemType = std::unique_ptr<void, void (*)(void *)>;

  public:
    ECS();

    inline auto &registry() { return *m_registry; }

    void init();

    template <typename TSystem> void createSystem()
    {
        uint32_t id = TypeID<ECS>::getID<TSystem>();
        RDE_ASSERT_0(id <= m_systems.size(),
                     "Some system is not added into systems container!");

        SystemType system{new TSystem{},
                          [](void *p) { delete static_cast<TSystem *>(p); }};
        m_systems.emplace_back(std::move(system));
    }

    template <typename TSystem> void registerSystem()
    {
        auto &delegate = m_updateDelegates.emplace_back();
        delegate.connect<&TSystem::update>(&getSystem<TSystem>());
    }

    void update(float dt);
    void createSystems();
    void registerSystems();

  private:
    template <typename TSystem> TSystem &getSystem()
    {
        uint32_t id = TypeID<ECS>::getID<TSystem>();
        RDE_ASSERT_0(id <= m_systems.size(),
                     "Some system is not added into systems container!");

        return *static_cast<TSystem *>(m_systems[id].get());
    }

    std::unique_ptr<entt::registry> m_registry;
    std::vector<entt::delegate<void(entt::registry &, float)>>
        m_updateDelegates;
    std::vector<SystemType> m_systems;
};
} // namespace RDE
