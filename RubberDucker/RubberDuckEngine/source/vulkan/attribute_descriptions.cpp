#include "precompiled/pch.hpp"
#include "attribute_descriptions.hpp"
#include "vulkan/binding_ids.hpp"

namespace RDE {
namespace Vulkan {

	RDE::Vulkan::AttributeDescriptions::AttributeDescriptions()
	{
		// Vertex
		VkVertexInputAttributeDescription posAttrDesc{};
		posAttrDesc.binding = VertexBufferBindingID;
		posAttrDesc.location = location++;
		posAttrDesc.format = VK_FORMAT_R32G32B32_SFLOAT;
		posAttrDesc.offset = offsetof(Vertex, pos);

		VkVertexInputAttributeDescription colorAttrDesc{};
		colorAttrDesc.binding = VertexBufferBindingID;
		colorAttrDesc.location = location++;
		colorAttrDesc.format = VK_FORMAT_R32G32B32_SFLOAT;
		colorAttrDesc.offset = offsetof(Vertex, color);

		VkVertexInputAttributeDescription texCoordAttrDesc{};
		texCoordAttrDesc.binding = VertexBufferBindingID;
		texCoordAttrDesc.location = location++;
		texCoordAttrDesc.format = VK_FORMAT_R32G32_SFLOAT;
		texCoordAttrDesc.offset = offsetof(Vertex, texCoord);

		vertex = { posAttrDesc, colorAttrDesc, texCoordAttrDesc };

		// Instance
		VkVertexInputAttributeDescription col0AttrDesc{};
		col0AttrDesc.binding = InstanceBufferBindingID;
		col0AttrDesc.location = location++;
		col0AttrDesc.format = VK_FORMAT_R32G32B32A32_SFLOAT;
		col0AttrDesc.offset = 0 * sizeof(glm::vec4);

		VkVertexInputAttributeDescription col1AttrDesc{};
		col1AttrDesc.binding = InstanceBufferBindingID;
		col1AttrDesc.location = location++;
		col1AttrDesc.format = VK_FORMAT_R32G32B32A32_SFLOAT;
		col1AttrDesc.offset = 1 * sizeof(glm::vec4);

		VkVertexInputAttributeDescription col2AttrDesc{};
		col2AttrDesc.binding = InstanceBufferBindingID;
		col2AttrDesc.location = location++;
		col2AttrDesc.format = VK_FORMAT_R32G32B32A32_SFLOAT;
		col2AttrDesc.offset = 2 * sizeof(glm::vec4);

		VkVertexInputAttributeDescription col3AttrDesc{};
		col3AttrDesc.binding = InstanceBufferBindingID;
		col3AttrDesc.location = location++;
		col3AttrDesc.format = VK_FORMAT_R32G32B32A32_SFLOAT;
		col3AttrDesc.offset = 3 * sizeof(glm::vec4);

		instance = { col0AttrDesc, col1AttrDesc, col2AttrDesc, col3AttrDesc };
	}
}
}
