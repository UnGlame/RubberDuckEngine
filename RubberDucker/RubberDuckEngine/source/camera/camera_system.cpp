#include "precompiled/pch.hpp"
#include "camera/camera_handler.hpp"
#include "camera_system.hpp"

namespace RDE {

	void CameraSystem::update(Engine* engine, float dt)
	{
        static auto& inputHandler = engine->inputHandler();
        static auto& camera = engine->scene().camera();
        static auto& cameraHandler = engine->cameraHandler();
        static auto& window = engine->window();

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
        }
        else {
            window.setCursorDisabled(false);
        }

	}
}