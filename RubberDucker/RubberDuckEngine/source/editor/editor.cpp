#include "precompiled/pch.hpp"

#include "editor/editor.hpp"

#include "core/main.hpp"
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

    showDockSpace();
    showHierarchy();
    showInspector();
    showDebugInfo();

    ImGui::ShowDemoWindow();
}

void Editor::newFrame() const
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();

    ImGui::NewFrame();
}

void Editor::showDockSpace() const
{
    // TODO: Render scene framebuffer onto ImGUI viewport - https://github.com/ocornut/imgui/issues/5110
    static bool open = true;
    static ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_PassthruCentralNode;

    // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
    // because it would be confusing to have two docking targets within each others.
    // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
    // and handle the pass-thru hole, so we ask Begin() to not render a background.
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
                                   ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                   ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground;

    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
    // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
    // all active windows docked into it will lose their parent and become undocked.
    // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
    // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
    ImGui::Begin("DockSpace Demo", &open, windowFlags);
    ImGui::PopStyleVar(3);

    // Submit the DockSpace
    const ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
        ImGuiID dockspace_id = ImGui::GetID("Dock Space");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspaceFlags);
    }

    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("Options")) {

            ImGui::Separator();
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    ImGui::End();
}

void Editor::showHierarchy() const
{
    ImGui::Begin("Hierarchy");

    ImGui::End();
}

void Editor::showInspector() const
{
    ImGui::Begin("Inspector");

    ImGui::End();
}

void Editor::showDebugInfo()
{
    ImGui::Begin("Debug Info");

    constexpr float interval = 0.25f;
    auto dt = g_engine->dt();
    m_dtTimer += dt;

    if (m_dtTimer >= interval) {
        m_dtToDisplay = dt;
        m_dtTimer = 0.0f;
    }
    const auto& io = ImGui::GetIO();
    ImGui::TextUnformatted(fmt::format("FPS: {}", static_cast<int32_t>(1 / m_dtToDisplay)).c_str());
    ImGui::Text("ImGUI frame time: %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
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
