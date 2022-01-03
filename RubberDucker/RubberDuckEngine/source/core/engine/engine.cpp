#include "precompiled/pch.hpp"
#include "core/engine/engine.hpp"
#include "utilities/clock/clock.hpp"

#include "core/data_structure/component_container.hpp"

namespace RDE {

    struct TestComponent
    {
        int integer = 0;

        bool operator==(const TestComponent& rhs) {
            return integer == rhs.integer;
        }
    };

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

        Clock clock;

        ComponentContainer<TestComponent> componentContainer;

        for (int i = 0; i < 10; ++i) {
            componentContainer.emplace(i, 10 - i);
        }

        componentContainer.forEach([this](Entity entity, const TestComponent& component) {
            RDE_LOG_DEBUG("Entity {0}, Component {1}", entity, component.integer);
        });

        for (int i = 0; i < 9; ++i) {
            componentContainer.remove(i);
        }

        componentContainer.forEach([this](Entity entity, const TestComponent& component) {
            RDE_LOG_DEBUG("Entity {0}, Component {1}", entity, component.integer);
            });

        Entity entity = componentContainer.find([this](const TestComponent& component) { return component.integer == 1; });
        RDE_LOG_DEBUG("entity found: {}", entity);
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