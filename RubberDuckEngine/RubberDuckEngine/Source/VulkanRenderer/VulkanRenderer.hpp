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
		[[nodiscard]] std::vector<VkExtensionProperties> retrieveSupportedExtensionsList();
		[[nodiscard]] bool checkValidationLayerSupport();
		void createInstance();

		const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
#ifdef RD_DEBUG
		const bool enableValidationLayers = false;
#else
		const bool enableValidationLayers = true;
#endif

		VkInstance m_instance;
		GLFWwindow* m_window;
	};
}