#pragma once
#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

namespace RDE {
namespace Vulkan {

	struct Instance
	{
		glm::mat4 modelTransform;
	};
}
}