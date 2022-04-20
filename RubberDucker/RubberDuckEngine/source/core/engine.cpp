#include "precompiled/pch.hpp"
#include "core/engine.hpp"
#include "utilities/clock.hpp"

namespace RDE {

    Engine::Engine() :
        m_renderer(std::make_unique<Vulkan::Renderer>()),
        m_ecs(std::make_unique<ECS>()),
        m_window(std::make_unique<Window>()),
        m_scene(std::make_unique<Scene>()),
        m_editor(std::make_unique<Editor>()),
        m_inputHandler(std::make_unique<InputHandler>()),
        m_cameraHandler(std::make_unique<CameraHandler>()),
        m_assetManager(std::make_unique<AssetManager>())
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
        m_renderer->init();
        m_editor->init();
        m_ecs->init();
        m_scene->init();
    }

    void Engine::mainLoop()
    {
        static auto* apiWindow = m_window->apiWindow();

        while (!m_shutdown && !glfwWindowShouldClose(apiWindow)) {
            m_deltaTime = Clock::deltaTime([this]() {
                glfwPollEvents();

                m_ecs->update(m_deltaTime);

                m_editor->update();
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