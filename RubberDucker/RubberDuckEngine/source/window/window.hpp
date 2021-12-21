#pragma once
#include <GLFW/glfw3.h>

namespace RDE
{
	class Window
	{
	public:
		static constexpr uint32_t width = 1600;
		static constexpr uint32_t height = 1200;
		
		void init();
		void cleanup();

		__forceinline GLFWwindow* get() { return m_GLFWwindow; }
	private:
		GLFWwindow* m_GLFWwindow;
	};
}