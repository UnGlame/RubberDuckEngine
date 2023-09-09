#include "precompiled/pch.hpp"

#include "hierarchy_system.hpp"

#include <imgui.h>
#include <string>

namespace RDE
{
	void HierarchySystem::update(entt::registry& registry, float dt)
	{
		static bool test = false;
		static std::string current_item;
		ImGui::Begin("Hierarchy", &test, ImGuiWindowFlags_::ImGuiWindowFlags_None);
		const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 650, main_viewport->WorkPos.y + 20), ImGuiCond_FirstUseEver);

		auto view = registry.view<TransformComponent>();

		ImGui::BeginListBox("##list", {-1, -1});
		view.each([&](auto entity, auto& transform) {
			std::string name = std::to_string(static_cast<int>(entity));
			bool is_selected = (current_item == name.c_str());
			if (ImGui::Selectable(name.c_str(), is_selected))
			{
				current_item = name.c_str();
			}
		});
		ImGui::EndListBox();

		ImGui::End();
	}
}