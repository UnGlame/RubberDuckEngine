#pragma once
#include "vulkan/vulkan.hpp"

namespace RDE {
namespace Vulkan {

	struct UniformBufferObject
	{
		alignas(16) glm::mat4 view;
		alignas(16) glm::mat4 projection;
	};
}
}