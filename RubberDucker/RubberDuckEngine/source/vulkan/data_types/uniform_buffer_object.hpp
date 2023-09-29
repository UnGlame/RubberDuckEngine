#pragma once
#include <glm/glm.hpp>

namespace RDE
{
namespace Vulkan
{

struct UniformBufferObject {
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 projection;
};
} // namespace Vulkan
} // namespace RDE