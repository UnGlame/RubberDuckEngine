#pragma once
#include "entt/entt.hpp"
#include "camera/camera.hpp"

namespace RDE {

	class Scene
	{
	public:
		Scene() = default;
		~Scene() = default;

		inline entt::registry& registry() { return m_registry; }
		inline const entt::registry& registry() const { return m_registry; }
		inline Camera& camera() { return m_camera; }
		inline const Camera& camera() const { return m_camera; }
	
	private:
		entt::registry m_registry;
		Camera m_camera;
	};
}