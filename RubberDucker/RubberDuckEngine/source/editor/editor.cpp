#include "editor/editor.hpp"
#include "precompiled/pch.hpp"
#include "vulkan/renderer.hpp"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include <spdlog/fmt/ostr.h>

namespace RDE
{

void Editor::init() {}

void Editor::update() {
    if (!m_renderingEnabled) {
        return;
    }

    newFrame();

    ImGui::ShowDemoWindow();

    debugInfo();
}

void Editor::newFrame() const {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();

    ImGui::NewFrame();
}

void Editor::debugInfo() {
    ImGui::Begin("Debug Info");

    constexpr float interval = 0.25f;
    auto dt = g_engine->dt();
    m_dtTimer += dt;

    if (m_dtTimer >= interval) {
        m_dtToDisplay = dt;
        m_dtTimer = 0.0f;
    }

    ImGui::Text(fmt::format("FPS: {}", static_cast<int32_t>(1 / m_dtToDisplay))
                    .c_str());

    ImGui::Separator();

    static auto &renderer = g_engine->renderer();
    ImGui::Text(
        fmt::format("Number of draw calls: {}", renderer.drawCallCount())
            .c_str());

    ImGui::End();
}
} // namespace RDE
