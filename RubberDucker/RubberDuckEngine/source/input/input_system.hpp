#pragma once
#include <entt/entt.hpp>

namespace RDE
{

class InputSystem
{
  public:
    void update(entt::registry& registry, float dt);
};
} // namespace RDE
