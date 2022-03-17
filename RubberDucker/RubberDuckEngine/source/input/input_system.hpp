#include "precompiled/pch.hpp"
#include "input/input_handler.hpp"
#include "utilities/utilities.hpp"

namespace RDE {

	class InputSystem
	{
	public:
		void update(entt::registry& registry, float dt);
	};
}