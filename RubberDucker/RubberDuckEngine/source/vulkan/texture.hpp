#pragma once
#include "vulkan/vulkan.hpp"

namespace RDE {
namespace Vulkan {

struct Texture {
    uint32_t mipLevels;

    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory imageMemory = VK_NULL_HANDLE;
    VkImageView imageView = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
};
} // namespace Vulkan
} // namespace RDE
