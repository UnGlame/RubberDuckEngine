#include "Pch.h"
#include "Window/Window.hpp"
#include "VulkanRenderer/VulkanRenderer.hpp"

namespace RD
{
    class HelloTriangleApplication
    {
    public:
        void run()
        {
            init();
            mainLoop();
            cleanup();
        }

    private:
        void init()
        {
            m_window.init();
            m_renderer.init(m_window.get());
        }

        void mainLoop()
        {
            while (!glfwWindowShouldClose(m_window.get()))
                glfwPollEvents();
        }

        void cleanup()
        {
            m_window.cleanup();
            m_renderer.cleanup();
        }

        Window m_window;
        VulkanRenderer m_renderer;
    };
}
int main()
{
    RD::HelloTriangleApplication app;

    try {
        app.run();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}