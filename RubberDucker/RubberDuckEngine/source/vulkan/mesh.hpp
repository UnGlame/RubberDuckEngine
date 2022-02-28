#pragma once
#include <vector>
#include "vulkan/vulkan.hpp"

namespace RDE {
namespace Vulkan {

	struct Vertex;

	struct Mesh
	{
		// Vertices and indices
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		// Vertex and Index buffers
		VkBuffer vertexBuffer = VK_NULL_HANDLE;
		VkDeviceMemory vertexBufferMemory = VK_NULL_HANDLE;
		VkBuffer indexBuffer = VK_NULL_HANDLE;
		VkDeviceMemory indexBufferMemory = VK_NULL_HANDLE;

		using VerticesValueType = decltype(vertices)::value_type;
		using IndicesValueType = decltype(indices)::value_type;
	};
}
}