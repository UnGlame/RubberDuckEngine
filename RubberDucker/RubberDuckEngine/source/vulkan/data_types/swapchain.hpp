#pragma once
#include <vulkan/vulkan.hpp>

namespace RDE
{
namespace Vulkan
{

struct Swapchain {
    struct SupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;

        [[nodiscard]] inline VkBool32 isAdequate() const
        {
            return !formats.empty() && !presentModes.empty();
        }
    };

    VkSwapchainKHR handle = VK_NULL_HANDLE;

    // Swapchain config
    VkFormat imageFormat;
    VkExtent2D extent;

    // Swap chain data
    std::vector<VkImage> images;
    std::vector<VkImageView> imageViews;
    std::vector<VkFramebuffer> framebuffers;

    inline size_t size() const
    {
        return framebuffers.size();
    }
};
} // namespace Vulkan
} // namespace RDE
