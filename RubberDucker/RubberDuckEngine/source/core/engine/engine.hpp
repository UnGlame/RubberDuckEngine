#pragma once
#include "window/window.hpp"
#include "vulkan/renderer/renderer.hpp"
#include "scene/scene.hpp"

namespace RDE {

    class Engine
    {
    public:
        Engine();
        void run();

        inline auto& window() { return *m_window; }
        inline auto& renderer() { return *m_renderer; }
        inline auto& scene() { return *m_scene; }

        inline float dt() { return m_deltaTime; } // Return deltaTime in seconds

    private:
        void init();
        void mainLoop();
        void cleanup();

        std::unique_ptr<Window> m_window;
        std::unique_ptr<Vulkan::Renderer> m_renderer;
        std::unique_ptr<Scene> m_scene;

        float m_deltaTime = 0;
    };
}