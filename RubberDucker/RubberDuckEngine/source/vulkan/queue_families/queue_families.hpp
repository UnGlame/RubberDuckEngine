#pragma once
#include <vulkan/vulkan.hpp>
#include <optional>

namespace RDE {
namespace Vulkan {

	struct QueueFamilyIndices
	{
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		[[nodiscard]] __forceinline VkBool32 isComplete() const {
			return graphicsFamily && presentFamily;
		}
	};
}
}