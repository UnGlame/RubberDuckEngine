#include "precompiled/pch.hpp"
#include "vulkan/systems/instance_update_system.hpp"
#include "vulkan/instance.hpp"

namespace RDE {

	void InstanceUpdateSystem::update(entt::registry& registry, float dt)
	{
		// Update instance buffers for each mesh
		auto& renderer = g_engine->renderer();
		auto group = registry.group<TransformComponent, ModelComponent>();

		renderer.clearMeshInstances();

		group.each([&](auto entity, auto& transform, auto& model) {
			// Copy transform into a new Instance
			glm::mat4 rotate(1.0f), scale(1.0f), translate(1.0f);
			scale = glm::scale(scale, transform.scale);
			rotate = glm::mat4_cast(transform.rotate);
			translate = glm::translate(translate, transform.translate);

			Vulkan::Instance instance;
			std::vector<Vulkan::Instance>& instances = renderer.getInstancesForMesh(model.modelGUID);
			instance.modelTransform = translate * rotate * scale;

			// Emplace Instance into a vector mapped from the mesh
			instances.emplace_back(std::move(instance));
		});
		
		// For each mesh, copy vector of Instance into big InstanceBuffers
		renderer.copyInstancesIntoInstanceBuffer();
	}
}