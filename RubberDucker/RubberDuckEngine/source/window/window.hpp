#pragma once
// This must be included before GLFW to prevent redefinition of APIENTRY
#include <Windows.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace RDE {

	class Window
	{
	public:
		void init();
		void cleanup();

		__forceinline GLFWwindow* get() { return m_GLFWwindow; }
		__forceinline bool isResized() { return m_resized; }
		__forceinline void setResized(bool resized) { m_resized = resized; }
		
		template <typename T>
		__forceinline T width() { return static_cast<T>(m_width); }
		
		template <typename T>
		__forceinline T height() { return static_cast<T>(m_height); }

		template <typename T>
		__forceinline void setWidth(T width) { m_width = static_cast<uint32_t>(width); }

		template <typename T>
		__forceinline void setHeight(T height) { m_height = static_cast<uint32_t>(height); }

		static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

	private:
		GLFWwindow* m_GLFWwindow;
		
		bool m_resized = false;

		uint32_t m_width = 800;
		uint32_t m_height = 600;
	};
}