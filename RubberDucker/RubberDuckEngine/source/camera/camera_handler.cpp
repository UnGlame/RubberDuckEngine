#include "camera/camera_handler.hpp"
#include "precompiled/pch.hpp"

namespace RDE
{

void CameraHandler::moveForward(Camera& camera, float dt)
{
    camera.eye = camera.eye + camera.front * camera.speed * dt;
}

void CameraHandler::moveBackward(Camera& camera, float dt)
{
    camera.eye = camera.eye - camera.front * camera.speed * dt;
}

void CameraHandler::moveLeft(Camera& camera, float dt)
{
    camera.eye = camera.eye - camera.right * camera.speed * dt;
}

void CameraHandler::moveRight(Camera& camera, float dt)
{
    camera.eye = camera.eye + camera.right * camera.speed * dt;
}

void CameraHandler::zoomIn(Camera& camera, float dt)
{
    camera.fov += camera.zoomSpeed * dt;
}

void CameraHandler::zoomOut(Camera& camera, float dt)
{
    camera.fov -= camera.zoomSpeed * dt;
}

void CameraHandler::computeVectors(Camera& camera)
{
    camera.pitch = glm::clamp(camera.pitch, -89.9f, 89.9f);
    glm::vec3 direction;
    direction.x =
        cosf(glm::radians(camera.yaw)) * cosf(glm::radians(camera.pitch));
    direction.y = sinf(glm::radians(camera.pitch));
    direction.z =
        sinf(glm::radians(camera.yaw)) * cosf(glm::radians(camera.pitch));

    camera.front = glm::normalize(direction);
    camera.right = glm::normalize(glm::cross(-camera.up, camera.front));
}
} // namespace RDE
