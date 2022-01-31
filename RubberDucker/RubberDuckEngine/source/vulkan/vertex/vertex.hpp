#pragma once
#include <vulkan/vulkan.hpp>

namespace RDE {
namespace Vulkan {

	struct Vertex
	{
		glm::vec3 pos;
		glm::vec3 color;
		glm::vec2 textureUV;

		static VkVertexInputBindingDescription getBindingDescription();
		static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions();
	};
}
}