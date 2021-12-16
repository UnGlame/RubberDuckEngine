#pragma once

namespace RD
{
	class VulkanRenderer
	{
	public:
		void init(GLFWwindow* window);
		void cleanup();
		
	private:
		[[nodiscard]] VkApplicationInfo createAppInfo();
		void createInstance();

		VkInstance m_instance;
		GLFWwindow* m_window;
	};
}