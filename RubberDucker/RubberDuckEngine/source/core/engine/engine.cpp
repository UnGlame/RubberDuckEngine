#include "precompiled/pch.hpp"
#include "core/engine/engine.hpp"
#include "utilities/clock/clock.hpp"

#include "core/data_structure/recyclable_sparse_set.hpp"

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

        Clock clock;
        RDE::RecyclableSparseSet < uint32_t, std::numeric_limits<uint32_t>::max(), 4096> sparseSet;

        for (auto i = 0; i < 3000000; ++i) {
            sparseSet.insert();
        }
        sparseSet.forEach([](uint32_t value) {
        });

        for (uint32_t i = 102145; i < 206533; ++i) {
            sparseSet.remove(i);
        }
        sparseSet.forEach([](uint32_t value) {
        });
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