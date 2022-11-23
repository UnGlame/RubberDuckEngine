#pragma once
#include "vma_image.hpp"

#include <vulkan/vulkan.hpp>

namespace RDE
{
namespace Vulkan
{

struct Texture {
    uint32_t mipLevels;

    VmaImage vmaImage{};
    VkImageView imageView = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;

    // Swapchain duplicates of the same descriptor sets
    std::vector<VkDescriptorSet> descriptorSets;
};
} // namespace Vulkan
} // namespace RDE
