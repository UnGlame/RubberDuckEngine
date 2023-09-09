#pragma once

#include <entt/entt.hpp>

namespace RDE
{
class Engine;

class CameraSystem
{
  public:
    void update(entt::registry& registry, float dt);
};
} // namespace RDE
