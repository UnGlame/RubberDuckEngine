#pragma once

namespace RDE
{
	class VulkanRenderer
	{
	public:
		void init(GLFWwindow* window);
		void cleanup();
		
	private:
		struct QueueFamilyIndices
		{
			uint32_t graphicsFamily;
		};

		// API-specific functions
		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
		
		VkResult createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
			const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);

		void destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
		
		// Utility functions
		[[nodiscard]] VkApplicationInfo createAppInfo() const;
		[[nodiscard]] std::vector<VkExtensionProperties> retrieveSupportedExtensionsList() const;
		[[nodiscard]] std::vector<const char*> retrieveRequiredExtensions() const;
		[[nodiscard]] bool checkValidationLayerSupport() const;
		[[nodiscard]] bool isDeviceSuitable(VkPhysicalDevice device) const;
		[[nodiscard]] QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) const;

		bool checkGlfwExtensions(const std::vector<VkExtensionProperties>& supportedExtensions, const std::vector<const char*>& glfwExtensions) const;
		void configureDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) const;
		void configureInstanceCreateInfo(VkInstanceCreateInfo& createInfo, const VkApplicationInfo& appInfo, 
			VkDebugUtilsMessengerCreateInfoEXT& debugMessengerCreateInfo, const std::vector<const char*>& glfwExtensions) const;
		
		void createInstance();
		void setupDebugMessenger();
		void selectPhysicalDevice();

#ifdef RDE_DEBUG
		const bool m_enableValidationLayers = true;
#else
		const bool m_enableValidationLayers = false;
#endif

		const std::vector<const char*> m_validationLayers = { "VK_LAYER_KHRONOS_validation" };

		VkInstance m_instance;
		VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
		VkDebugUtilsMessengerEXT m_debugMessenger;
		VkAllocationCallbacks* m_allocator = nullptr;

		GLFWwindow* m_window;
	};
}