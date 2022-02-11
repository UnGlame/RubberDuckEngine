#include "precompiled/pch.hpp"
#include "input_system.hpp"

namespace RDE {

	void InputSystem::update(Engine* engine, float dt)
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
        }
        else {
            window.setCursorDisabled(false);
        }

        if (inputHandler.isKeyPressed(KeyCode::F11)) {
            window.toggleDisplayType();
        }

		
		inputHandler.clearAllPressedInput();
		inputHandler.computeMouseDelta();	
	}
}