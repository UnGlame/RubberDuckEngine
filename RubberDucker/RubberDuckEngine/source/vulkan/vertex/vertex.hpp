#pragma once
#include <vulkan/vulkan.hpp>

namespace RDE {
namespace Vulkan {

	struct Vertex
	{
		glm::vec2 pos;
		glm::vec3 color;

		static VkVertexInputBindingDescription getBindingDescription();
		static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions();
	};
}
}