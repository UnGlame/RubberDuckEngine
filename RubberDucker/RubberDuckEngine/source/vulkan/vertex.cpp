#include "pch.hpp"
#include "vertex.hpp"

namespace RDE
{
namespace Vulkan
{
	VkVertexInputBindingDescription Vertex::getBindingDescription()
	{
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}
}
}