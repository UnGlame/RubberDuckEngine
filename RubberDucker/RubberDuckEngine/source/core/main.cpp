#include "pch.hpp"
#include "window/window.hpp"
#include "renderer/vulkan_renderer.hpp"

namespace RDE
{
    class HelloTriangleApplication
    {
    public:
        HelloTriangleApplication() :
            m_window(std::make_unique<Window>()),
            m_renderer(std::make_unique<VulkanRenderer>())
        {}

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
            m_window->init();;
            m_renderer->init(m_window->get());
        }

        void mainLoop()
        {
            while (!glfwWindowShouldClose(m_window->get())) {
                Clock::framesPerSecond([this]() {
                    glfwPollEvents();
                    m_renderer->drawFrame();
                });
            }

            m_renderer->waitForOperations();
        }

        void cleanup()
        {
            m_window->cleanup();
            m_renderer->cleanup();
        }

        std::unique_ptr<Window> m_window;
        std::unique_ptr<VulkanRenderer> m_renderer;
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
        RDE_LOG_CRITICAL("Exception thrown: {}", e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}