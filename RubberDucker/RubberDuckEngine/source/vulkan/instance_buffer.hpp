#pragma once
#include "vulkan/vma_buffer.hpp"

#include <vulkan/vulkan.hpp>

namespace RDE
{
namespace Vulkan
{

struct InstanceBuffer {
    VmaBuffer vmaBuffer{};
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkDescriptorBufferInfo descriptor{};

    VmaBuffer stagingBuffer{};

    VkDeviceSize size = 0;
    uint32_t instanceCount = 0;
};
} // namespace Vulkan
} // namespace RDE
