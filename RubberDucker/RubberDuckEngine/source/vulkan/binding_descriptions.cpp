#include "precompiled/pch.hpp"
#include "vulkan/binding_descriptions.hpp"
#include "vulkan/binding_ids.hpp"

namespace RDE
{
namespace Vulkan
{

BindingDescriptions::BindingDescriptions() : vertex{}, instance{}
{
    vertex.binding = VertexBufferBindingID;
    vertex.stride = sizeof(Vertex);
    vertex.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    instance.binding = InstanceBufferBindingID;
    instance.stride = sizeof(Instance);
    instance.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
}
} // namespace Vulkan
} // namespace RDE
