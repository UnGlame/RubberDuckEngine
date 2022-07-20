#include "precompiled/pch.hpp"

#include "camera/camera_handler.hpp"
#include "camera/camera_system.hpp"

namespace RDE
{

void CameraSystem::update(entt::registry &registry, float dt) {
    static auto &inputHandler = g_engine->inputHandler();
    static auto &camera = g_engine->scene().camera();
    static auto &cameraHandler = g_engine->cameraHandler();
    static auto &window = g_engine->window();

    if (inputHandler.isMouseKeyDown(MouseCode::Mouse2)) {
        window.setCursorDisabled(true);

        if (inputHandler.isKeyDown(KeyCode::W)) {
            cameraHandler.moveForward(camera, dt);
        }
        if (inputHandler.isKeyDown(KeyCode::A)) {
            cameraHandler.moveLeft(camera, dt);
        }
        if (inputHandler.isKeyDown(KeyCode::S)) {
            cameraHandler.moveBackward(camera, dt);
        }
        if (inputHandler.isKeyDown(KeyCode::D)) {
            cameraHandler.moveRight(camera, dt);
        }
        glm::ivec2 mouseDelta = inputHandler.rawMouseDelta();
        float sensitivity = 0.25f;
        camera.yaw += mouseDelta.x * sensitivity;
        camera.pitch += mouseDelta.y * sensitivity;
        cameraHandler.computeVectors(camera);
    } else {
        window.setCursorDisabled(false);
    }
}
} // namespace RDE
