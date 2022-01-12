#pragma once
#include "entt/entt.hpp"

namespace RDE {

	class Scene
	{
	public:
		Scene() = default;
		~Scene() = default;

		inline entt::registry& getRegistry() { return m_registry; }
	private:
		entt::registry m_registry;
	};
}