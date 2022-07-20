#pragma once
#include <glm/mat4x4.hpp>

namespace RDE {
namespace Vulkan {

struct PushConstantObject {
    glm::mat4 modelMtx;
};
} // namespace Vulkan
} // namespace RDE
