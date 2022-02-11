#include "precompiled/pch.hpp"
#include "input/input_handler.hpp"
#include "utilities/utilities.hpp"

namespace RDE {

	InputHandler::KeyBitset InputHandler::s_keyDown{};
	InputHandler::KeyBitset InputHandler::s_keyPressed{};

	InputHandler::MouseBitset InputHandler::s_mouseDown{};
	InputHandler::MouseBitset InputHandler::s_mousePressed{};

	glm::vec2 InputHandler::s_mousePos{ 0 };
	glm::vec2 InputHandler::s_prevMousePos{ 0 };

	void InputHandler::keyInputCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
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

	void InputHandler::mouseInputCallback(GLFWwindow* window, int button, int action, int mods)
	{
		if (action == GLFW_PRESS) {
			s_mousePressed.set(button);
			s_mouseDown.set(button);
		}
		else if (action == GLFW_RELEASE) {
			s_mouseDown.reset(button);
		}
	}

	void InputHandler::mousePositionCallback(GLFWwindow* window, double xpos, double ypos)
	{
		s_prevMousePos = s_mousePos;
		s_mousePos = static_cast<glm::vec2>(glm::dvec2{ xpos, ypos });
	}

	void InputHandler::computeMouseDelta()
	{
		auto componentMouseDelta = [](float current, float previous) -> uint32_t {
			return Utilities::floatEqual(current, previous) ? 0 : (current - previous) > 0.0f ? 1 : -1;
		};

		m_mouseDelta = { componentMouseDelta(s_mousePos.x, s_prevMousePos.x), componentMouseDelta(s_mousePos.y, s_prevMousePos.y) };
	}
}