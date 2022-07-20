#pragma once
#include <vulkan/vulkan.hpp>

namespace RDE {
namespace Vulkan {

struct InstanceBuffer {
    VkBuffer buffer = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkDescriptorBufferInfo descriptor;

    VkBuffer stagingBuffer = VK_NULL_HANDLE;
    VkDeviceMemory stagingBufferMemory = VK_NULL_HANDLE;

    VkDeviceSize size = 0;
    uint32_t instanceCount = 0;
};
} // namespace Vulkan
} // namespace RDE
