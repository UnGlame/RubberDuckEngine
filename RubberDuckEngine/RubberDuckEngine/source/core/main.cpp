#include "pch.hpp"
#include "window/window.hpp"
#include "renderer/vulkan_renderer.hpp"
#include "logger/logger.h"

namespace RDE
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
            Logger::init();
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
    // Enable run-time memory check for debug builds.
#if defined(RDE_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    RDE::HelloTriangleApplication app;

    try {
        app.run();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}