#include "Pch.h"
#include "VulkanRenderer/VulkanRenderer.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace RD
{
	void VulkanRenderer::init(GLFWwindow* window)
	{
		m_window = window;

		createInstance();
	}

	void VulkanRenderer::cleanup()
	{
		vkDestroyInstance(m_instance, nullptr);
	}

	VkApplicationInfo VulkanRenderer::createAppInfo()
	{
		VkApplicationInfo app_info{};
		app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		app_info.pApplicationName = "Hello Triangle";
		app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		app_info.pEngineName = "Rubber Duck Engine";
		app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		app_info.apiVersion = VK_API_VERSION_1_0;

		return app_info;
	}

	void VulkanRenderer::createInstance()
	{
		auto app_info = createAppInfo();

		VkInstanceCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		create_info.pApplicationInfo = &app_info;

		uint32_t glfwExtensionCount = 0;
		uint32_t glfwLayerCount = 0;

		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		
		create_info.enabledExtensionCount = glfwExtensionCount;
		create_info.ppEnabledExtensionNames = glfwExtensions;
		create_info.enabledLayerCount = glfwLayerCount;

		if (vkCreateInstance(&create_info, nullptr, &m_instance) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create Vk instance!");
		}
	}
}