#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

namespace RDE
{

struct TransformComponent {
    TransformComponent() = default;

    glm::quat rotate = glm::quat(1, 0, 0, 0);
    glm::vec3 scale = glm::vec3(1);
    glm::vec3 translate = glm::vec3(0);
};
} // namespace RDE