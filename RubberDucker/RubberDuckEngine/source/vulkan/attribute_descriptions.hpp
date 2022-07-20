#pragma once
#include "vulkan/instance.hpp"
#include "vulkan/vertex.hpp"
#include <vulkan/vulkan.hpp>

namespace RDE
{
namespace Vulkan
{

class AttributeDescriptions
{
  public:
    AttributeDescriptions();
    inline std::array<VkVertexInputAttributeDescription, 3>
    getVertexAttributeDescriptions() const
    {
        return vertex;
    }
    inline std::array<VkVertexInputAttributeDescription, 4>
    getInstanceAttributeDescriptions() const
    {
        return instance;
    }

  private:
    std::array<VkVertexInputAttributeDescription, 3> vertex;
    std::array<VkVertexInputAttributeDescription, 4> instance;

    uint32_t location = 0;
};
} // namespace Vulkan
} // namespace RDE
