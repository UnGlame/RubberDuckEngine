#pragma once
#include <entt/entt.hpp>

namespace RDE
{
	class HierarchySystem
	{
	public:
		void update(entt::registry& registry, float dt);
	};
}