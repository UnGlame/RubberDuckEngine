#pragma once
#include <entt/entt.hpp>

namespace RDE {

class InstanceUpdateSystem
{
public:
    void update(entt::registry& registry, float dt);
};
} // namespace RDE
