#include "precompiled/pch.hpp"
#include "core/engine.hpp"
#include "utilities/clock.hpp"

#include "components/transform_component.hpp"

namespace RDE {

    Engine::Engine() :
        m_window(std::make_unique<Window>()),
        m_scene(std::make_unique<Scene>()),
        m_renderer(std::make_unique<Vulkan::Renderer>()),
        m_inputHandler(std::make_unique<InputHandler>()),
        m_cameraHandler(std::make_unique<CameraHandler>())
    {}

    void Engine::run()
    {
        init();
        mainLoop();
        cleanup();
    }

    void Engine::init()
    {
        Logger::init();
        m_window->init();
        m_renderer->init(m_window.get());

        Clock clock;
    }

    void Engine::mainLoop()
    {
        auto* apiWindow = m_window->get();

        static auto& registry = m_scene->registry();

        auto entity = registry.create();
        auto& transform = registry.emplace<TransformComponent>(entity);

        static float time = 0.0f;
        static bool reverse = false;

        while (!glfwWindowShouldClose(apiWindow)) {
            m_deltaTime = Clock::deltaTime([this, &transform]() {
                glfwPollEvents();

                static auto& camera = m_scene->camera();

                if (m_inputHandler->isKeyDown(KeyCode::W)) {
                    m_cameraHandler->moveForward(camera, m_deltaTime);
                }
                if (m_inputHandler->isKeyDown(KeyCode::A)) {
                    m_cameraHandler->moveLeft(camera, m_deltaTime);
                }
                if (m_inputHandler->isKeyDown(KeyCode::S)) {
                    m_cameraHandler->moveBackward(camera, m_deltaTime);
                }
                if (m_inputHandler->isKeyDown(KeyCode::D)) {
                    m_cameraHandler->moveRight(camera, m_deltaTime);
                }

                m_renderer->drawFrame();
            });
        }

        m_renderer->waitForOperations();
    }

    void Engine::cleanup()
    {
        m_window->cleanup();
        m_renderer->cleanup();
    }
}