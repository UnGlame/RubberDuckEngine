#include "precompiled/pch.hpp"
#include "input/input_handler.hpp"

namespace RDE {

	InputHandler::KeyBitset InputHandler::s_keyDown{};
	InputHandler::KeyBitset InputHandler::s_keyPressed{};

	void InputHandler::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_UNKNOWN) {
			return;
		}

		if (action == GLFW_PRESS) {
			s_keyPressed.set(key);
			s_keyDown.set(key);
		}
		else if (action == GLFW_RELEASE) {
			s_keyDown.reset(key);
		}
	}
}