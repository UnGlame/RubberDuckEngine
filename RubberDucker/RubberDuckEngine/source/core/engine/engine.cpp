#include "precompiled/pch.hpp"
#include "core/engine/engine.hpp"
#include "utilities/clock/clock.hpp"

#include "components/transform_component.hpp"

namespace RDE {

    Engine::Engine() :
        m_window(std::make_unique<Window>()),
        m_renderer(std::make_unique<Vulkan::Renderer>()),
        m_scene(std::make_unique<Scene>())
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

        static auto& registry = m_scene->getRegistry();

        auto entity = registry.create();
        auto& transform = registry.emplace<TransformComponent>(entity);

        static float time = 0.0f;
        static bool reverse = false;

        while (!glfwWindowShouldClose(apiWindow)) {
            m_deltaTime = Clock::deltaTime([this, &transform]() {
                glfwPollEvents();
                
                static const glm::quat start = glm::angleAxis(glm::radians(-60.0f), glm::vec3(0.0f, 0.0f, 1.0f));
                static const glm::quat end = glm::angleAxis(glm::radians(60.0f), glm::vec3(0.0f, 0.0f, 1.0f));
                static const glm::vec3 startPos = glm::vec3(-1.5f, 0.0f, 0.0f);
                static const glm::vec3 endPos = glm::vec3(1.5f, 0.0f, 0.0f);
                
                //transform.translate = glm::mix(startPos, endPos, reverse ? 1.0f - time : time);
                //transform.rotate = glm::slerp(start, end, reverse ? 1.0f - time : time);
                time += m_deltaTime;

                transform.rotate = glm::rotate(transform.rotate, glm::radians(90.0f) * m_deltaTime, glm::vec3(0.0f, 1.0f, 1.0f));

                if (time > 1.0f) {
                    time = 0.0f;
                    reverse = !reverse;
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