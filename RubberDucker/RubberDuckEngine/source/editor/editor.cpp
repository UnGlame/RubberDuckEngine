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

void Editor::update()
{
    if (!m_renderingEnabled) {
        return;
    }

    newFrame();

    ImGui::ShowDemoWindow();

    showHierarchy();
    showInspector();
    showDebugInfo();
}

void Editor::newFrame() const
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();

    ImGui::NewFrame();
}

void Editor::showHierarchy()
{
    ImGui::Begin("Hierarchy");

    ImGui::End();
}

void Editor::showInspector()
{
    ImGui::Begin("Inspector");

    ImGui::End();
}

void Editor::showDebugInfo()
{
    ImGui::Begin("Show Debug Info");

    constexpr float interval = 0.25f;
    auto dt = g_engine->dt();
    m_dtTimer += dt;

    if (m_dtTimer >= interval) {
        m_dtToDisplay = dt;
        m_dtTimer = 0.0f;
    }

    ImGui::TextUnformatted(fmt::format("FPS: {}", static_cast<int32_t>(1 / m_dtToDisplay)).c_str());

    ImGui::Separator();

    static auto& renderer = g_engine->renderer();
    ImGui::TextUnformatted(fmt::format("Number of draw calls: {}", renderer.drawCallCount()).c_str());

    ImGui::Separator();

    const auto& instances = renderer.instancesString();
    for (const auto& [mesh, texture, instanceCount] : instances) {
        ImGui::TextWrapped("Drawing %s using %s with %d instances", mesh.c_str(), texture.c_str(), instanceCount);
    }
    ImGui::End();
}
} // namespace RDE
