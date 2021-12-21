#pragma once
#include "window/window.hpp"
#include "renderer/vulkan_renderer.hpp"

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
        std::unique_ptr<VulkanRenderer> m_renderer;
    };
}