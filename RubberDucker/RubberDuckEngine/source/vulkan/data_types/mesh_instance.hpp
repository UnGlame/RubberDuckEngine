#pragma once
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

namespace RDE
{
namespace Vulkan
{

struct MeshInstance {
    glm::mat4 modelTransform;
};
} // namespace Vulkan
} // namespace RDE
