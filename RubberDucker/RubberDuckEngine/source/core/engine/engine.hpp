#pragma once
#include "window/window.hpp"
#include "vulkan/renderer/renderer.hpp"

namespace RDE {

    class Engine
    {
    public:
        Engine();
        void run();

        inline Window& window() { return *m_window; }
        inline Vulkan::Renderer& renderer() { return *m_renderer; }
        inline float dt() { return m_deltaTime; } // Return deltaTime in seconds

    private:
        void init();
        void mainLoop();
        void cleanup();

        std::unique_ptr<Window> m_window;
        std::unique_ptr<Vulkan::Renderer> m_renderer;
        float m_deltaTime = 0;
    };
}