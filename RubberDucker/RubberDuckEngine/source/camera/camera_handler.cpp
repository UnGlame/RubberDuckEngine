#include "precompiled/pch.hpp"
#include "camera/camera_handler.hpp"

namespace RDE {

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

    void CameraHandler::rotate(Camera& camera, float dt)
    {

    }
}