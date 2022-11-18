#pragma once
#include <glm/glm.hpp>

namespace RDE
{

struct Camera;

class CameraHandler
{
  public:
    void moveForward(Camera& camera, float dt);
    void moveBackward(Camera& camera, float dt);
    void moveLeft(Camera& camera, float dt);
    void moveRight(Camera& camera, float dt);
    void zoomIn(Camera& camera, float dt);
    void zoomOut(Camera& camera, float dt);
    void computeVectors(Camera& camera);
};
} // namespace RDE
