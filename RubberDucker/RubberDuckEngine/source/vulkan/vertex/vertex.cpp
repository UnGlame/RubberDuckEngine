#include "precompiled/pch.hpp"
#include "vertex.hpp"

namespace RDE {
namespace Vulkan {

	VkVertexInputBindingDescription Vertex::getBindingDescription()
	{
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	std::array<VkVertexInputAttributeDescription, 3> Vertex::getAttributeDescriptions() {
		VkVertexInputAttributeDescription posAttrDescriptions{};
		posAttrDescriptions.binding = 0;
		posAttrDescriptions.location = 0;
		posAttrDescriptions.format = VK_FORMAT_R32G32B32_SFLOAT;
		posAttrDescriptions.offset = offsetof(Vertex, pos);

		VkVertexInputAttributeDescription colorAttrDescriptions{};
		colorAttrDescriptions.binding = 0;
		colorAttrDescriptions.location = 1;
		colorAttrDescriptions.format = VK_FORMAT_R32G32B32_SFLOAT;
		colorAttrDescriptions.offset = offsetof(Vertex, color);
		
		VkVertexInputAttributeDescription uvAttrDescriptions{};
		uvAttrDescriptions.binding = 0;
		uvAttrDescriptions.location = 2;
		uvAttrDescriptions.format = VK_FORMAT_R32G32_SFLOAT;
		uvAttrDescriptions.offset = offsetof(Vertex, texCoord);

		std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{ posAttrDescriptions, colorAttrDescriptions, uvAttrDescriptions };

		return attributeDescriptions;
	}
}
}