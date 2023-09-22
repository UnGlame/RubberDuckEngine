#include "precompiled/pch.hpp"

#include "editor/editor.hpp"

#include "assetmanager/asset_manager.hpp"
#include "core/main.hpp"
#include "ecs/components/component_list.hpp"
#include "vulkan/renderer.hpp"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include <rttr/registration>
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

    const auto types = rttr::type::get_types();

    for (const auto& type : types) {
        RDELOG_INFO("type: {}", type.get_name());
    }

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

void Editor::showHierarchy()
{
    ImGui::Begin("Hierarchy");
    auto& registry = g_engine->registry();

    if (ImGui::Button("Add entity")) {
        const auto entity = registry.create();
        const auto& camera = g_engine->scene().camera();
        constexpr float spawnDistance = 5.0f;
        auto& transform = registry.emplace<TransformComponent>(entity);
        transform.translate = camera.eye + camera.front * spawnDistance;
    }
    ImGui::Separator();

    registry.each([&](auto entity) {
        bool selected = entity == m_selected_entity;
        if (ImGui::Selectable(fmt::format("Entity {}", entity).c_str(), &selected)) {
            if (selected) {
                m_selected_entity = entity;

                // Reset euler angles to entity's quaternion
                // Only do this once, so that we do not repeat converting quat -> euler -> quat.
                auto* transform = registry.try_get<TransformComponent>(m_selected_entity);
                if (transform) {
                    m_eulerAngles = glm::degrees(glm::eulerAngles(transform->rotate));
                }
            }
        }
    });
    ImGui::End();
}

void Editor::showInspector()
{
    if (m_selected_entity == entt::null) {
        return;
    }
    auto& registry = g_engine->registry();
    auto& assetManager = g_engine->assetManager();
    ImGui::Begin("Inspector");

    // TODO: Use reflection to get component names
    std::vector<std::string> componentNames{"MeshComponent"};
    auto& currentComponent = componentNames[0];

    ImGui::Text("Add Component");
    if (ImGui::BeginCombo("##Add Component", "Transform Component")) {
        for (const auto& componentName : componentNames) {
            bool selected = currentComponent == componentName;

            if (ImGui::Selectable(componentName.c_str(), selected)) {
                currentComponent = componentName;

                if (componentName == "MeshComponent") {
                    auto& model = registry.emplace<MeshComponent>(m_selected_entity);
                    model.modelGuid = assetManager.getModelId("cube.obj");
                    model.textureGuid = assetManager.getTextureId("cube.png");
                }
            }
            if (selected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    ImGui::Separator();

    auto* transform = registry.try_get<TransformComponent>(m_selected_entity);

    if (transform) {
        if (ImGui::TreeNode("Transform Component")) {
            ImGui::PushItemWidth(60.0f);
            ImGui::Text("Scale");
            ImGui::DragFloat("x##scale", &transform->scale[0], 0.1f);
            ImGui::SameLine();
            ImGui::DragFloat("y##scale", &transform->scale[1], 0.1f);
            ImGui::SameLine();
            ImGui::DragFloat("z##scale", &transform->scale[2], 0.1f);

            ImGui::Text("Rotate");

            bool rotateChanged = false;
            rotateChanged = ImGui::DragFloat("x##rotate", &m_eulerAngles[0], 0.1f);
            ImGui::SameLine();
            rotateChanged |= ImGui::DragFloat("y##rotate", &m_eulerAngles[1], 0.1f);
            ImGui::SameLine();
            rotateChanged |= ImGui::DragFloat("z##rotate", &m_eulerAngles[2], 0.1f);

            if (rotateChanged) {
                transform->rotate = glm::quat(glm::radians(m_eulerAngles));
            }

            ImGui::Text("Translate");
            ImGui::DragFloat("x##translate", &transform->translate[0], 0.1f);
            ImGui::SameLine();
            ImGui::DragFloat("y##translate", &transform->translate[1], 0.1f);
            ImGui::SameLine();
            ImGui::DragFloat("z##translate", &transform->translate[2], 0.1f);
            ImGui::PopItemWidth();
            ImGui::TreePop();
        }
    }

    auto* model = registry.try_get<MeshComponent>(m_selected_entity);

    if (model) {
        if (ImGui::TreeNode("Mesh Component")) {
            ImGui::PushItemWidth(100.0f);
            const auto modelId = model->modelGuid;
            const auto modelNames = assetManager.getModelNames();
            auto currentModel = assetManager.getAssetName(modelId);

            if (ImGui::BeginCombo("Model Name", currentModel.c_str())) {
                for (const auto& modelName : modelNames) {
                    bool selected = currentModel == modelName;

                    if (ImGui::Selectable(modelName.c_str(), selected)) {
                        currentModel = modelName;
                        model->modelGuid = assetManager.getModelId(modelName.c_str());
                    }
                    if (selected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }

            const auto textureId = model->textureGuid;
            const auto textureNames = assetManager.getTextureNames();
            auto currentTexture = assetManager.getAssetName(textureId);

            if (ImGui::BeginCombo("Texture Name", currentTexture.c_str())) {
                for (const auto& textureName : textureNames) {
                    bool selected = currentTexture == textureName;

                    if (ImGui::Selectable(textureName.c_str(), selected)) {
                        currentTexture = textureName;
                        model->textureGuid = assetManager.getTextureId(textureName.c_str());
                    }
                    if (selected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }
            ImGui::PopItemWidth();
            ImGui::TreePop();
        }
    }
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
