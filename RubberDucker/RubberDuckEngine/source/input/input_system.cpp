#include "precompiled/pch.hpp"

#include "input_system.hpp"

#include "core/main.hpp"
#include "ecs/components/component_list.hpp"
#include "utilities/utilities.hpp"

namespace RDE {

void InputSystem::update(entt::registry& registry, float dt)
{
    static auto& inputHandler = g_engine->inputHandler();
    static auto& window = g_engine->window();

    static bool firstFrame = true;

    if (firstFrame) {
        firstFrame = false;
    } else {
        inputHandler.computeRawMouseDelta();
    }

    if (inputHandler.isKeyPressed(KeyCode::F11)) {
        window.toggleDisplayType();
    }
    if (inputHandler.isKeyPressed(KeyCode::F10)) {
        g_engine->editor().toggle();
    }
    if (inputHandler.isKeyPressed(KeyCode::Escape)) {
        g_engine->shutdown();
    }

    // Entity movement
    if (!inputHandler.isMouseKeyDown(MouseCode::Mouse2)) {
        if (inputHandler.isKeyDown(KeyCode::W)) {
            auto view = registry.view<TransformComponent>();
            view.each([=](auto entity, auto& transform) {
                glm::vec3 front = glm::rotate(transform.rotate, glm::vec3(0.0f, 0.0f, -1.0f));

                transform.translate = transform.translate + front * dt;
            });
        }
        if (inputHandler.isKeyDown(KeyCode::A)) {
            auto view = registry.view<TransformComponent>();
            view.each([=](auto entity, auto& transform) {
                glm::vec3 right = glm::rotate(transform.rotate, glm::vec3(1.0f, 0.0f, 0.0f));

                transform.translate = transform.translate - right * dt;
            });
        }
        if (inputHandler.isKeyDown(KeyCode::S)) {
            auto view = registry.view<TransformComponent>();
            view.each([=](auto entity, auto& transform) {
                glm::vec3 front = glm::rotate(transform.rotate, glm::vec3(0.0f, 0.0f, -1.0f));

                transform.translate = transform.translate - front * dt;
            });
        }
        if (inputHandler.isKeyDown(KeyCode::D)) {
            auto view = registry.view<TransformComponent>();
            view.each([=](auto entity, auto& transform) {
                glm::vec3 right = glm::rotate(transform.rotate, glm::vec3(1.0f, 0.0f, 0.0f));

                transform.translate = transform.translate + right * dt;
            });
        }
    }

    // Remove last entity
    if (inputHandler.isKeyDown(KeyCode::R)) {
        auto view = registry.view<TransformComponent>();
        auto entity = view.back();
        if (entity != entt::null) {
            registry.destroy(entity);
        }
    }

    if (inputHandler.isKeyPressed(KeyCode::L)) {
        auto& sceneManager = g_engine->sceneManager();
        sceneManager.saveCurrentScene();
    }

    inputHandler.resetInput();
}
} // namespace RDE
