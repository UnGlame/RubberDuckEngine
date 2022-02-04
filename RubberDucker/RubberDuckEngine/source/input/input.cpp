#include "precompiled/pch.hpp"
#include "input/input.hpp"

namespace RDE {

	void InputManager::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		switch (key) {
		case GLFW_KEY_F11: {
			if (action == GLFW_PRESS) {
				auto& window = g_engine->window();

				window.toggleFullscreen();
			}
			break;
		}
		default: {
			break;
		}
		}
	}
}