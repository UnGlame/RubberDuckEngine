#pragma once
#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>

namespace RDE
{
namespace Vulkan
{

struct VmaImage {
    VkImage buffer;
    VmaAllocation allocation;
    VmaAllocationInfo allocationInfo;
};

} // namespace Vulkan
} // namespace RDE
