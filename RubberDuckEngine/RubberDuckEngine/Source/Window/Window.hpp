#pragma once
#include <GLFW/glfw3.h>

namespace RDE
{
	class Window
	{
	public:
		static constexpr uint32_t width = 800;
		static constexpr uint32_t height = 600;
		
		void init();
		void cleanup();

		__forceinline GLFWwindow* get() { return m_GLFWwindow; }
	private:
		GLFWwindow* m_GLFWwindow;
	};
}