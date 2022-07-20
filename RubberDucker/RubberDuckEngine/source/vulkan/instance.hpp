#pragma once
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <vulkan/vulkan.hpp>

namespace RDE
{
namespace Vulkan
{

struct Instance {
  glm::mat4 modelTransform;
};
} // namespace Vulkan
} // namespace RDE
