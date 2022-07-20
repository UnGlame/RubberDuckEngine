#pragma once
#include <optional>
#include <vulkan/vulkan.hpp>

namespace RDE
{
namespace Vulkan
{

struct QueueFamilyIndices {
  std::optional<uint32_t> graphicsFamily;
  std::optional<uint32_t> presentFamily;

  [[nodiscard]] inline VkBool32 isComplete() const
  {
    return graphicsFamily && presentFamily;
  }
};
} // namespace Vulkan
} // namespace RDE
