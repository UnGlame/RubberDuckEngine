#pragma once
#include <vulkan/vulkan.hpp>

namespace RDE {
namespace Vulkan {

	struct InstanceBuffer
	{
		VkBuffer buffer = VK_NULL_HANDLE;
		VkDeviceMemory memory = VK_NULL_HANDLE;
		VkDescriptorBufferInfo descriptor;

		VkDeviceSize size = 0;
		uint32_t instanceCount = 1;
	};
}
}