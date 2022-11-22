#include "precompiled/pch.hpp"

#include "vulkan/instance.hpp"
#include "vulkan/systems/instance_update_system.hpp"

namespace RDE
{

void InstanceUpdateSystem::update(entt::registry& registry, float dt)
{
    // Update instance buffers for each mesh
    auto& renderer = g_engine->renderer();
    auto group = registry.group<TransformComponent, ModelComponent>();

    renderer.clearMeshInstances();

    group.each([&](auto entity, auto& transform, auto& model) {
        // Copy transform into a new Instance
        glm::mat4 modelMtx(1.0f);
        modelMtx = glm::translate(modelMtx, transform.translate) * glm::mat4_cast(transform.rotate) * glm::scale(modelMtx, transform.scale);

        Vulkan::Instance instance;
        std::vector<Vulkan::Instance>& instances = renderer.getInstancesForMesh(model.modelGUID, model.textureGUID);
        instance.modelTransform = modelMtx;

        // Move instance into a vector mapped from mesh and texture IDs
        instances.emplace_back(std::move(instance));
    });

    // For each mesh, copy vector of Instance into big InstanceBuffers
    renderer.copyInstancesIntoInstanceBuffer();
}
} // namespace RDE
