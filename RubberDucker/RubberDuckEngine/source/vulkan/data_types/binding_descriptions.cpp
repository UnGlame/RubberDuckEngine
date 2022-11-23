#include "precompiled/pch.hpp"

#include "binding_descriptions.hpp"
#include "binding_ids.hpp"
#include "mesh_instance.hpp"
#include "vertex.hpp"

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
    instance.stride = sizeof(MeshInstance);
    instance.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
}
} // namespace Vulkan
} // namespace RDE
