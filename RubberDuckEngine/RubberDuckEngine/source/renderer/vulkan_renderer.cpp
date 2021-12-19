#include "pch.hpp"
#include "renderer/vulkan_renderer.hpp"
namespace RDE
{
	void VulkanRenderer::init(GLFWwindow* window)
	{
		m_window = window;
		createInstance();
		setupDebugMessenger();
		createSurface();
		selectPhysicalDevice();
		createLogicalDevice();

		RDE_LOG_INFO("End");
	}

	void VulkanRenderer::cleanup()
	{
		vkDestroyDevice(m_device, m_allocator);

		if (m_enableValidationLayers) {
			destroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, m_allocator);
		}

		vkDestroySurfaceKHR(m_instance, m_surface, m_allocator);
		vkDestroyInstance(m_instance, nullptr);
	}

	VKAPI_ATTR VkBool32 VKAPI_CALL VulkanRenderer::debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData)
	{
		switch (messageSeverity) {
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: {
				RDE_LOG_WARN("Validation Layer:\n\t{}", pCallbackData->pMessage);
				break;
			}
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: {
				RDE_LOG_ERROR("Validation Layer:\n\t{}", pCallbackData->pMessage);
				break;
			}
			default: {
				//RDE_LOG_INFO("Validation Layer:\n\t{}", pCallbackData->pMessage);
				break;
			}
		}
		return VK_FALSE;
	}

	// Our own function wrapped around the API extension function since it needs to be loaded from its address
	VkResult VulkanRenderer::createDebugUtilsMessengerEXT(
		VkInstance instance,
		const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator,
		VkDebugUtilsMessengerEXT* pDebugMessenger)
	{
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

		if (func != nullptr) {
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}
		else {
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	void VulkanRenderer::destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
	{
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

		if (func != nullptr) {
			func(instance, debugMessenger, pAllocator);
		}
	}

	[[nodiscard]]
	VkApplicationInfo VulkanRenderer::createAppInfo() const
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
	std::vector<VkExtensionProperties> VulkanRenderer::retrieveSupportedExtensionsList() const
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

		std::ostringstream ss;
		ss << "\nAvailable extensions: \n";

		for (const auto& extension : extensions) {
			ss << '\t' << extension.extensionName << '\n';
		}
		RDE_LOG_INFO(ss.str());

		return extensions;
	}

	[[nodiscard]]
	std::vector<const char*> VulkanRenderer::retrieveRequiredExtensions() const {
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		if (m_enableValidationLayers) {
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}
		return extensions;
	}

	[[nodiscard]]
	bool VulkanRenderer::checkValidationLayerSupport() const
	{
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (const char* layerName : m_validationLayers) {
			bool layerFound = false;

			for (const auto& layerProperties : availableLayers) {
				if (strcmp(layerName, layerProperties.layerName) == 0) {
					layerFound = true;
					break;
				}
			}

			if (!layerFound) {
				return false;
			}
		}
		return true;
	}

	[[nodiscard]]
	bool VulkanRenderer::isDeviceSuitable(VkPhysicalDevice device) const
	{
		bool suitableDeviceType = false;
		bool suitableDeviceFeatures = false;
		bool suitableQueueFamily = false;

		VkPhysicalDeviceProperties deviceProperties;
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceProperties(device, &deviceProperties);
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

		// Discete GPU that (high performance)
		suitableDeviceType = deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
		// Supports geometry shaders
		suitableDeviceFeatures = deviceFeatures.geometryShader;
		// Suitable queue family
		suitableQueueFamily = findQueueFamilies(device).isComplete();

		return suitableDeviceType && suitableDeviceFeatures && suitableQueueFamily;
	}

	[[nodiscard]]
	VulkanRenderer::QueueFamilyIndices VulkanRenderer::findQueueFamilies(VkPhysicalDevice device) const
	{
		QueueFamilyIndices indices{};

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		int index = 0;
		for (const auto& queueFamily : queueFamilies) {
			// Queue family supports graphics operations
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				indices.graphicsFamily = index;
			}
			// Device supports present
			VkBool32 presentSupport = false;
			
			if (vkGetPhysicalDeviceSurfaceSupportKHR(device, index, m_surface, &presentSupport) != VK_SUCCESS) {
				throw std::runtime_error("Failed to get surface present support!");
			}

			if (presentSupport) {
				indices.presentFamily = index;
			}

			if (indices.isComplete()) {
				break;
			}
			++index;
		}

		return indices;
	}

	[[nodiscard]]
	bool VulkanRenderer::checkGlfwExtensions(
		const std::vector<VkExtensionProperties>& supportedExtensions,
		const std::vector<const char*>& glfwExtensions) const
	{
		for (const auto& extension : supportedExtensions) {
			static bool extensionFound = false;

			for (const char* glfwExtension : glfwExtensions) {
				if (strcmp(extension.extensionName, glfwExtension) == 0) {
					extensionFound = true;
					break;
				}
			}

			if (!extensionFound) {
				return false;
			}
		}
		return true;
	}

	void VulkanRenderer::configureDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) const
	{
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity =
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType =
			VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = debugCallback;
		createInfo.pUserData = nullptr;
	}

	void VulkanRenderer::configureInstanceCreateInfo(VkInstanceCreateInfo& createInfo, const VkApplicationInfo& appInfo,
		VkDebugUtilsMessengerCreateInfoEXT& debugMessengerCreateInfo, const std::vector<const char*>& glfwExtensions) const
	{
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(glfwExtensions.size());
		createInfo.ppEnabledExtensionNames = glfwExtensions.data();

		if (m_enableValidationLayers) {
			if (!checkValidationLayerSupport()) {
				throw std::runtime_error("Validation layers requested but not available!");
			}

			createInfo.enabledLayerCount = static_cast<uint32_t>(m_validationLayers.size());
			createInfo.ppEnabledLayerNames = m_validationLayers.data();

			// Setup debug messenger for instance creation and destruction
			configureDebugMessengerCreateInfo(debugMessengerCreateInfo);
			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugMessengerCreateInfo;
		}
		else {
			createInfo.enabledLayerCount = 0;
			createInfo.pNext = nullptr;
		}
	}

	void VulkanRenderer::createInstance()
	{
		// Create app info and instance create info
		auto appInfo = createAppInfo();

		VkInstanceCreateInfo createInfo{};
		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};

		const auto glfwExtensions = retrieveRequiredExtensions();

		// Configure create info
		configureInstanceCreateInfo(createInfo, appInfo, debugCreateInfo, glfwExtensions);

		// Create instance
		if (vkCreateInstance(&createInfo, m_allocator, &m_instance) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create Vk instance!");
		}

		// Retrieve supported extensions and check against glfw extensions
		const auto supportedExtensions = retrieveSupportedExtensionsList();
	}

	void VulkanRenderer::setupDebugMessenger()
	{
		if (!m_enableValidationLayers) return;

		VkDebugUtilsMessengerCreateInfoEXT createInfo{};
		configureDebugMessengerCreateInfo(createInfo);

		if (createDebugUtilsMessengerEXT(m_instance, &createInfo, m_allocator, &m_debugMessenger) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create debug utils messenger!");
		}
	}

	void VulkanRenderer::createSurface()
	{
		if (glfwCreateWindowSurface(m_instance, m_window, m_allocator, &m_surface) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create window surface!");
		}
	}

	void VulkanRenderer::selectPhysicalDevice()
	{
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);

		if (deviceCount == 0) {
			throw std::runtime_error("Failed to find GPUs with Vulkan support!");
		}

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());

		for (const auto& device : devices) {
			// Select first GPU device to use
			if (isDeviceSuitable(device)) {
				m_physicalDevice = device;
				break;
			}
		}

		// If at the end variable is still null, no device is suitable
		if (m_physicalDevice == VK_NULL_HANDLE) {
			throw std::runtime_error("Failed to find a suitable GPU device!");
		}
	}

	void VulkanRenderer::createLogicalDevice()
	{
		QueueFamilyIndices indices = findQueueFamilies(m_physicalDevice);

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::unordered_set<uint32_t> uniqueQueueFamilies = {
			indices.graphicsFamily.value(), indices.presentFamily.value()
		};

		float queuePriority = 1.0f;
		for (auto queueFamily : uniqueQueueFamilies) {
			VkDeviceQueueCreateInfo queueCreateInfo{};

			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1; // Will only ever need 1
			queueCreateInfo.pQueuePriorities = &queuePriority;
			
			queueCreateInfos.emplace_back(std::move(queueCreateInfo));
		}

		VkPhysicalDeviceFeatures deviceFeatures{};
		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pEnabledFeatures = &deviceFeatures;
		createInfo.enabledExtensionCount = 0;

		// No longer needed but added to be compatible with older versions
		if (m_enableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(m_validationLayers.size());
			createInfo.ppEnabledLayerNames = m_validationLayers.data();
		}
		else {
			createInfo.enabledLayerCount = 0;
		}

		if (vkCreateDevice(m_physicalDevice, &createInfo, m_allocator, &m_device) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create logical device!");
		}

		// Cache device queue
		vkGetDeviceQueue(m_device, indices.graphicsFamily.value(), 0, &m_graphicsQueue);
	}
}