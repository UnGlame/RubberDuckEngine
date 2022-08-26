#pragma once
#include "camera/camera.hpp"

namespace RDE
{

class Scene
{
  public:
    Scene() = default;
    ~Scene() = default;

    void init();

    inline Camera& camera() { return m_camera; }
    inline const Camera& camera() const { return m_camera; }

  private:
    Camera m_camera;
};
} // namespace RDE
