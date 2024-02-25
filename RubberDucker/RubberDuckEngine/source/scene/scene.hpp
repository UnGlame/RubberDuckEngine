#pragma once
#include "camera/camera.hpp"

#include <entt/entt.hpp>

namespace RDE {

class Scene
{
public:
    Scene();
    Scene(const Scene& rhs) = delete;
    Scene(Scene&& rhs);
    Scene& operator=(const Scene& rhs) = delete;
    Scene& operator=(Scene&& rhs);
    ~Scene() = default;

    void init();
    void cleanup();

    Camera& camera();
    const Camera& camera() const;
    entt::registry& registry();
    entt::registry& registry() const;

private:
    std::unique_ptr<entt::registry> m_registry;
    std::unique_ptr<Camera> m_camera;
};
} // namespace RDE
