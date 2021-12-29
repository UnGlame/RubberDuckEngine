#include "precompiled/pch.hpp"
#include "core/engine/engine.hpp"
#include "utilities/clock/clock.hpp"

namespace RDE {

    Engine::Engine() :
        m_window(std::make_unique<Window>()),
        m_renderer(std::make_unique<Vulkan::Renderer>())
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
    }

    void Engine::mainLoop()
    {
        auto* apiWindow = m_window->get();

        while (!glfwWindowShouldClose(apiWindow)) {
            m_deltaTime = Clock::deltaTime([this]() {
                glfwPollEvents();
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