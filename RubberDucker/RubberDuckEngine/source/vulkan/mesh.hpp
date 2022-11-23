#pragma once
#include "vulkan/instance_buffer.hpp"
#include "vulkan/vulkan.hpp"
#include <vector>

namespace RDE
{
namespace Vulkan
{

struct Vertex;

struct Mesh {
    // Vertices and indices
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    // Vertex and Index buffers
    VmaBuffer vertexBuffer{};
    VmaBuffer indexBuffer{};
    std::unordered_map<uint32_t, InstanceBuffer> instanceBuffers{};

    using VerticesValueType = decltype(vertices)::value_type;
    using IndicesValueType = decltype(indices)::value_type;
};
} // namespace Vulkan
} // namespace RDE
