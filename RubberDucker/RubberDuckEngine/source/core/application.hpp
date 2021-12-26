#pragma once
#include "window/window.hpp"
#include "vulkan/renderer.hpp"

namespace RDE
{
    class Application
    {
    public:
        Application();
        void run();

    private:
        void init();
        void mainLoop();
        void cleanup();

        std::unique_ptr<Window> m_window;
        std::unique_ptr<Vulkan::Renderer> m_renderer;
    };
}