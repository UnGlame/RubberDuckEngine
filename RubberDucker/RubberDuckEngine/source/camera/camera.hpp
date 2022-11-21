#pragma once
#include <glm/glm.hpp>

namespace RDE
{

struct Camera {
    glm::vec3 eye = {0.0f, 0.0f, 3.0f};
    glm::vec3 front = {0.0f, 0.0f, -1.0f};
    glm::vec3 right = {1.0f, 0.0f, 0.0f};
    glm::vec3 up = {0.0f, 1.0f, 0.0f};

    float fov = 90.0f;
    float nearClip = 0.0001f;
    float farClip = 10000.0f;
    float speed = 40.0f;
    float zoomSpeed = 20.0f;
    float pitch = 0.0f;
    float yaw = -90.0f;
};
} // namespace RDE
