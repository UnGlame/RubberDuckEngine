#pragma once


namespace RDE {
	class Engine;

	class CameraSystem
	{
	public:
		void update(entt::registry& registry, float dt);
	};
}