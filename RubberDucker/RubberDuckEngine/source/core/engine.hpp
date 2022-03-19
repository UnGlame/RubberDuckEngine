#pragma once
#include "assetmanager/asset_manager.hpp"
#include "camera/camera_handler.hpp"
#include "ecs/ecs.hpp"
#include "editor/editor.hpp"
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

        inline auto& renderer() { return *m_renderer; }
        inline auto& window() { return *m_window; }
        inline auto& scene() { return *m_scene; }
        inline auto& registry() { return m_ecs->registry(); }
        inline auto& editor() { return *m_editor; }
        inline auto& inputHandler() { return *m_inputHandler; }
        inline auto& cameraHandler() { return *m_cameraHandler; }
        inline auto& assetManager() { return *m_assetManager; }

        inline float dt() { return m_deltaTime; } // Return deltaTime in seconds

    private:
        void init();
        void mainLoop();
        void cleanup();

        std::unique_ptr<Vulkan::Renderer> m_renderer;
        std::unique_ptr<Window> m_window;
        std::unique_ptr<Scene> m_scene;
        std::unique_ptr<ECS> m_ecs;
        std::unique_ptr<Editor> m_editor;

        std::unique_ptr<InputHandler> m_inputHandler;
        std::unique_ptr<CameraHandler> m_cameraHandler;
        std::unique_ptr<AssetManager> m_assetManager;

        float m_deltaTime = 0;
    };
}