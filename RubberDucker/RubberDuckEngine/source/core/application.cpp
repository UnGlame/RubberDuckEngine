#include "pch.hpp"
#include "application.hpp"

namespace RDE
{
    Application::Application() :
        m_window(std::make_unique<Window>()),
        m_renderer(std::make_unique<Vulkan::Renderer>())
    {}

    void Application::run()
    {
        init();
        mainLoop();
        cleanup();
    }

    void Application::init()
    {
        Logger::init();
        m_window->init();
        m_renderer->init(m_window.get());
    }

    void Application::mainLoop()
    {
        while (!glfwWindowShouldClose(m_window->get())) {
            Clock::framesPerSecond([this]() {
                glfwPollEvents();
                m_renderer->drawFrame();
            });
        }

        m_renderer->waitForOperations();
    }

    void Application::cleanup()
    {
        m_window->cleanup();
        m_renderer->cleanup();
    }
}