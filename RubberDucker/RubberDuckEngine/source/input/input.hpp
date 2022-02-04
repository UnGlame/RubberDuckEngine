#pragma once
#include "window/window.hpp"

namespace RDE {

	class InputManager
	{
	public:
		// Callback used to retrieve key input from glfw
		static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

	private:
	};
}