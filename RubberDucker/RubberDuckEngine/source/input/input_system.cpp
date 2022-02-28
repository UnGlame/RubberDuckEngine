#include "precompiled/pch.hpp"
#include "input_system.hpp"

namespace RDE {

	void InputSystem::update(Engine* engine, float dt)
	{
        static auto& inputHandler = engine->inputHandler();
        static auto& window = engine->window();
		static auto& registry = engine->registry();

		static bool firstFrame = true;

		if (firstFrame) {
			firstFrame = false;
		}
		else {
			inputHandler.computeRawMouseDelta();
		}

        if (inputHandler.isKeyPressed(KeyCode::F11)) {
            window.toggleDisplayType();
        }
		if (inputHandler.isKeyDown(KeyCode::W)) {
			auto& view = registry.view<TransformComponent>();
			view.each([=](auto entity, auto& transform) {
				glm::vec3 front = glm::rotate(transform.rotate, glm::vec3(0.0f, 0.0f, -1.0f));
				
				transform.translate = transform.translate + front * dt;
			});
		}if (inputHandler.isKeyDown(KeyCode::A)) {
			auto& view = registry.view<TransformComponent>();
			view.each([=](auto entity, auto& transform) {
				glm::vec3 right = glm::rotate(transform.rotate, glm::vec3(1.0f, 0.0f, 0.0f));
				
				transform.translate = transform.translate - right * dt;
			});
		}if (inputHandler.isKeyDown(KeyCode::S)) {
			auto& view = registry.view<TransformComponent>();
			view.each([=](auto entity, auto& transform) {
				glm::vec3 front = glm::rotate(transform.rotate, glm::vec3(0.0f, 0.0f, -1.0f));
				
				transform.translate = transform.translate - front * dt;
			});
		}if (inputHandler.isKeyDown(KeyCode::D)) {
			auto& view = registry.view<TransformComponent>();
			view.each([=](auto entity, auto& transform) {
				glm::vec3 right = glm::rotate(transform.rotate, glm::vec3(1.0f, 0.0f, 0.0f));
				
				transform.translate = transform.translate + right * dt;
			});
		}

        inputHandler.resetInput();
	}
}