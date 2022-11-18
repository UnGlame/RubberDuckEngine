#pragma once
#include <vulkan/vulkan.hpp>
#include <entt/entt.hpp>

namespace RDE {

	class InstanceUpdateSystem
	{
	public:
		void update(entt::registry& registry, float dt);
	};
}