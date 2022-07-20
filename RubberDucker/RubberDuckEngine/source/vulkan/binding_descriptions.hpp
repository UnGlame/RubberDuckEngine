#pragma once
#include "vulkan/instance.hpp"
#include "vulkan/vertex.hpp"
#include <vulkan/vulkan.hpp>

namespace RDE
{
namespace Vulkan
{

class BindingDescriptions
{
  public:
    BindingDescriptions();
    inline VkVertexInputBindingDescription getVertexBindingDescription() const
    {
        return vertex;
    }
    inline VkVertexInputBindingDescription getInstanceBindingDescription() const
    {
        return instance;
    }

  private:
    VkVertexInputBindingDescription vertex;
    VkVertexInputBindingDescription instance;
};
} // namespace Vulkan
} // namespace RDE
