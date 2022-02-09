#pragma once
#include "camera/camera_handler.hpp"
#include "input/input_handler.hpp"
#include "scene/scene.hpp"
#include "vulkan/renderer.hpp"
#include "window/window.hpp"

namespace RDE {

    class Engine
    {
    public:
        Engine();
        void run();

        inline auto& scene() { return *m_scene; }
        inline auto& window() { return *m_window; }
        inline auto& renderer() { return *m_renderer; }
        inline auto& inputHandler() { return *m_inputHandler; }
        inline auto& cameraHandler() { return *m_cameraHandler; }

        inline float dt() { return m_deltaTime; } // Return deltaTime in seconds

    private:
        void init();
        void mainLoop();
        void cleanup();

        std::unique_ptr<Scene> m_scene;
        std::unique_ptr<Window> m_window;
        std::unique_ptr<Vulkan::Renderer> m_renderer;
        std::unique_ptr<InputHandler> m_inputHandler;
        std::unique_ptr<CameraHandler> m_cameraHandler;

        float m_deltaTime = 0;
    };
}