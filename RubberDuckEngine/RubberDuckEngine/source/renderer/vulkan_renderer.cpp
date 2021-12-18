#include "pch.hpp"
#include "renderer/vulkan_renderer.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "logger/Logger.h"

namespace RDE
{
	void VulkanRenderer::init(GLFWwindow* window)
	{
		m_window = window;
		Logger::test();
		createInstance();
	}

	void VulkanRenderer::cleanup()
	{
		vkDestroyInstance(m_instance, nullptr);
	}

	[[nodiscard]]
	VkApplicationInfo VulkanRenderer::createAppInfo()
	{
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Hello Triangle";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "Rubber Duck Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		return appInfo;
	}

	[[nodiscard]]
	std::vector<VkExtensionProperties> VulkanRenderer::retrieveSupportedExtensionsList()
	{
		uint32_t extensionCount = 0;

		// Retrieve number of supported extensions
		if (vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr) != VK_SUCCESS) {
			throw std::runtime_error("Failed to retrieve instance extension count!");
		}

		std::vector<VkExtensionProperties> extensions(extensionCount);

		// Query extension details
		if (vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data()) != VK_SUCCESS) {
			throw std::runtime_error("Failed to retrieve instance extension list!");
		}

		return extensions;
	}

	bool VulkanRenderer::checkGlfwExtensions(const std::vector<VkExtensionProperties>& supportedExtensions, const char** glfwExtensions, uint32_t glfwExtensionsCount)
	{
		for (const auto& extension : supportedExtensions) {
			static bool extensionFound = false;

			for (uint32_t i = 0; i < glfwExtensionsCount; ++i) {
				if (strcmp(extension.extensionName, glfwExtensions[i]) == 0) {
					extensionFound = true;
					break;
				}
			}

			if (!extensionFound) {
				RDE_LOG_WARN("Missing extension: {0}", extension.extensionName);
				return false;
			}
		}
		return true;
	}

	bool VulkanRenderer::checkValidationLayerSupport()
	{
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);

		return true;
	}

	void VulkanRenderer::createInstance()
	{
		auto appInfo = createAppInfo();

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		uint32_t glfwExtensionCount = 0;
		uint32_t glfwLayerCount = 0;

		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		createInfo.enabledExtensionCount = glfwExtensionCount;
		createInfo.ppEnabledExtensionNames = glfwExtensions;
		createInfo.enabledLayerCount = glfwLayerCount;

		if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create Vk instance!");
		}

		const auto supportedExtensions = retrieveSupportedExtensionsList();

		std::ostringstream ss;
		ss << "\nAvailable extensions: \n";

		for (const auto& extension : supportedExtensions) {
			ss << '\t' << extension.extensionName << '\n';
		}
		RDE_LOG_INFO(ss.str());

		checkGlfwExtensions(supportedExtensions, glfwExtensions, glfwExtensionCount);
	}

}