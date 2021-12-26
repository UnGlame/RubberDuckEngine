#include "pch.hpp"
#include "vulkan/renderer.hpp"

namespace RDE {
namespace Vulkan {

	void Renderer::init(Window* window)
	{
		RDE_LOG_INFO("Start");
		m_window = window;

		createInstance();
		setupDebugMessenger();
		createSurface();
		selectPhysicalDevice();
		createLogicalDevice();
		createSwapchain();
		createImageViews();
		createRenderPass();
		createGraphicsPipeline();
		createFramebuffers();
		createCommandPool();
		createCommandBuffers();
		createSynchronizationObjects();
		
		RDE_LOG_INFO("End");
	}

	void Renderer::drawFrame()
	{
		// Wait for fence at (previous) frame
		vkWaitForFences(m_device, 1, &m_inFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX);

		// Acquire image from swap chain
		uint32_t imageIndex;
		auto result = vkAcquireNextImageKHR(m_device, m_swapchain, UINT64_MAX, m_imageAvailableSemaphores[m_currentFrame], VK_NULL_HANDLE, &imageIndex); // UINT64_MAX disables timeout

		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			recreateSwapchain();
			return;
		}
		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			RDE_LOG_CRITICAL("Failed to acquire next swap chain image!");
		}

		// If previous frame is using this image, we need to wait for its fence
		if (m_imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
			vkWaitForFences(m_device, 1, &m_imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
		}

		// Mark image as being in use by this frame
		m_imagesInFlight[imageIndex] = m_inFlightFences[m_currentFrame];

		// Execute command buffer with image as attachment in the framebuffer
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = { m_imageAvailableSemaphores[m_currentFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT }; // Each stage corresponds to each wait semaphore
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &m_commandBuffers[imageIndex];

		VkSemaphore signalSemaphores[] = { m_renderFinishedSemaphores[m_currentFrame] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		vkResetFences(m_device, 1, &m_inFlightFences[m_currentFrame]);

		if (vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, m_inFlightFences[m_currentFrame]) != VK_SUCCESS) {
			RDE_LOG_CRITICAL("Failed to submit draw command buffer!");
		}

		// Return image to swap chain for presentation
		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores; // Wait for render to finish
		
		VkSwapchainKHR swapchains[] = { m_swapchain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapchains;
		presentInfo.pImageIndices = &imageIndex;
		presentInfo.pResults = nullptr;

		result = vkQueuePresentKHR(m_presentQueue, &presentInfo);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_window->isResized()) {
			recreateSwapchain();
			m_window->setResized(false);
		}
		else if (result != VK_SUCCESS) {
			RDE_LOG_CRITICAL("Failed to present swapchain image!");
		}

		m_currentFrame = (m_currentFrame + 1) % c_maxFramesInFlight;
	}

	void Renderer::cleanup()
	{
		cleanupSwapchain();

		for (uint32_t i = 0; i < c_maxFramesInFlight; ++i) {
			vkDestroySemaphore(m_device, m_imageAvailableSemaphores[i], m_allocator);
			vkDestroySemaphore(m_device, m_renderFinishedSemaphores[i], m_allocator);
			vkDestroyFence(m_device, m_inFlightFences[i], m_allocator);
		}

		vkDestroyCommandPool(m_device, m_commandPool, m_allocator);

		vkDestroyDevice(m_device, m_allocator);

		if (c_enableValidationLayers) {
			destroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, m_allocator);
		}

		vkDestroySurfaceKHR(m_instance, m_surface, m_allocator);
		vkDestroyInstance(m_instance, nullptr);
	}

	VKAPI_ATTR VkBool32 VKAPI_CALL Renderer::debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData)
	{
		switch (messageSeverity) {
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: {
				RDE_LOG_CLEAN_WARN(pCallbackData->pMessage);
				break;
			}
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: {
				RDE_LOG_CLEAN_ERROR(pCallbackData->pMessage);
				break;
			}
			default: {
				//RDE_LOG_CLEAN_INFO("Validation Layer:\n\t{}", pCallbackData->pMessage);
				break;
			}
		}
		return VK_FALSE;
	}

	// Our own function wrapped around the API extension function since it needs to be loaded from its address
	VkResult Renderer::createDebugUtilsMessengerEXT(
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

	void Renderer::destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
	{
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

		if (func != nullptr) {
			func(instance, debugMessenger, pAllocator);
		}
	}

	[[nodiscard]]
	VkShaderModule Renderer::createShaderModule(FileIO::FileBufferType shaderCode) const
	{
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = shaderCode.size();
		// Each 4 chars of code is stored as 1 uint32_t
		createInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode.data());

		VkShaderModule shaderModule;
		if (vkCreateShaderModule(m_device, &createInfo, m_allocator, &shaderModule) != VK_SUCCESS) {
			RDE_LOG_CRITICAL("Failed to create shader module!");
		}
		return shaderModule;
	}

	[[nodiscard]]
	std::vector<VkExtensionProperties> Renderer::retrieveSupportedExtensionsList() const
	{
		uint32_t extensionCount = 0;

		// Retrieve number of supported extensions
		if (vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr) != VK_SUCCESS) {
			RDE_LOG_CRITICAL("Failed to retrieve instance extension count!");
		}

		std::vector<VkExtensionProperties> extensions(extensionCount);

		// Query extension details
		if (vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data()) != VK_SUCCESS) {
			RDE_LOG_CRITICAL("Failed to retrieve instance extension list!");
		}

		//std::ostringstream ss;
		//ss << "\nAvailable extensions: \n";
		//
		//for (const auto& extension : extensions) {
		//	ss << '\t' << extension.extensionName << '\n';
		//}
		//RDE_LOG_INFO(ss.str());

		return extensions;
	}

	[[nodiscard]]
	std::vector<const char*> Renderer::retrieveRequiredExtensions() const {
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		if (c_enableValidationLayers) {
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}
		return extensions;
	}

	[[nodiscard]]
	VkBool32 Renderer::checkGlfwExtensions(
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

	[[nodiscard]]
	VkBool32 Renderer::checkValidationLayerSupport() const
	{
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (const char* layerName : c_validationLayers) {
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

	VkBool32 Renderer::checkDeviceExtensionSupport(VkPhysicalDevice device) const
	{
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		std::unordered_set<std::string> requiredExtensions(c_deviceExtensions.begin(), c_deviceExtensions.end());

		// Eliminate required extension off the checklist for all available ones
		for (const auto& extension : availableExtensions) {
			requiredExtensions.erase(extension.extensionName);
		}

		// If required extensions are empty, it means that available extensions checked them off
		return requiredExtensions.empty();
	}

	[[nodiscard]]
	VkBool32 Renderer::isDeviceSuitable(VkPhysicalDevice device) const
	{
		VkBool32 suitable = true;

		VkPhysicalDeviceProperties deviceProperties;
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceProperties(device, &deviceProperties);
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

		// Device is discrete GPU (high performance)
		suitable &= deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
		// Device supports geometry shaders
		suitable &= deviceFeatures.geometryShader;
		// Device has suitable queue family
		suitable &= queryQueueFamilies(device).isComplete();
		// Device supports extensions
		suitable &= checkDeviceExtensionSupport(device);
		// Device has adequate swap chain support
		if (suitable) {
			// Can only query if device extension is available
			SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
			suitable &= swapChainSupport.isAdequate();
		}

		return suitable;
	}

	[[nodiscard]]
	QueueFamilyIndices Renderer::queryQueueFamilies(VkPhysicalDevice device) const
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
				RDE_LOG_CRITICAL("Failed to get surface present support!");
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

	Renderer::SwapChainSupportDetails Renderer::querySwapChainSupport(VkPhysicalDevice device) const
	{
		SwapChainSupportDetails details;

		// Surface Capabilites
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_surface, &details.capabilities);

		// Surface formats
		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, nullptr);

		if (formatCount) {
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, details.formats.data());
		}

		// Presentation modes
		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, nullptr);

		if (formatCount) {
			details.presentModes.resize(formatCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &formatCount, details.presentModes.data());
		}

		return details;
	}

	VkSurfaceFormatKHR Renderer::selectSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) const
	{
		for (const auto& availableFormat : availableFormats) {
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				return availableFormat;
			}
		}

		// If SRGB is not available, just use the first format for simplicity
		return availableFormats[0];
	}

	VkPresentModeKHR Renderer::selectSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) const
	{
		for (const auto& availablePresentMode : availablePresentModes) {
			// Allows triple buffering
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR){
				return availablePresentMode;
			}
		}

		// FIFO is guaranteed to be available
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D Renderer::selectSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const
	{
		if (capabilities.currentExtent.width != UINT32_MAX) {
			return capabilities.currentExtent;
		}

		int width, height;
		// Retrieve width and height in pixels
		glfwGetFramebufferSize(m_window->get(), &width, &height);

		VkExtent2D actualExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		// Ensure extents are within min and max of capabilities
		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actualExtent;
	}

	void Renderer::configureDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) const
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

	void Renderer::configureInstanceCreateInfo(VkInstanceCreateInfo& createInfo, const VkApplicationInfo& appInfo,
		VkDebugUtilsMessengerCreateInfoEXT& debugMessengerCreateInfo, const std::vector<const char*>& glfwExtensions) const
	{
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(glfwExtensions.size());
		createInfo.ppEnabledExtensionNames = glfwExtensions.data();

		if (c_enableValidationLayers) {
			if (!checkValidationLayerSupport()) {
				RDE_LOG_CRITICAL("Validation layers requested but not available!");
			}

			createInfo.enabledLayerCount = static_cast<uint32_t>(c_validationLayers.size());
			createInfo.ppEnabledLayerNames = c_validationLayers.data();

			// Setup debug messenger for instance creation and destruction
			configureDebugMessengerCreateInfo(debugMessengerCreateInfo);
			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugMessengerCreateInfo;
		}
		else {
			createInfo.enabledLayerCount = 0;
			createInfo.pNext = nullptr;
		}
	}

	void Renderer::createInstance()
	{
		RDE_PROFILE_SCOPE

		// Create app info and instance create info
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Hello Triangle";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "Rubber Duck Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		VkInstanceCreateInfo createInfo{};
		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};

		const auto glfwExtensions = retrieveRequiredExtensions();

		// Configure create info
		configureInstanceCreateInfo(createInfo, appInfo, debugCreateInfo, glfwExtensions);

		// Create instance
		if (vkCreateInstance(&createInfo, m_allocator, &m_instance) != VK_SUCCESS) {
			RDE_LOG_CRITICAL("Failed to create Vk instance!");
		}

		// Retrieve supported extensions and check against glfw extensions
		const auto supportedExtensions = retrieveSupportedExtensionsList();
	}

	void Renderer::setupDebugMessenger()
	{
		RDE_PROFILE_SCOPE

		if (!c_enableValidationLayers) return;

		VkDebugUtilsMessengerCreateInfoEXT createInfo{};
		configureDebugMessengerCreateInfo(createInfo);

		if (createDebugUtilsMessengerEXT(m_instance, &createInfo, m_allocator, &m_debugMessenger) != VK_SUCCESS) {
			RDE_LOG_CRITICAL("Failed to create debug utils messenger!");
		}
	}

	void Renderer::createSurface()
	{
		RDE_PROFILE_SCOPE

		if (glfwCreateWindowSurface(m_instance, m_window->get(), m_allocator, &m_surface) != VK_SUCCESS) {
			RDE_LOG_CRITICAL("Failed to create window surface!");
		}
	}

	void Renderer::selectPhysicalDevice()
	{
		RDE_PROFILE_SCOPE

		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);

		if (deviceCount == 0) {
			RDE_LOG_CRITICAL("Failed to find GPUs with Vulkan support!");
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
			RDE_LOG_CRITICAL("Failed to find a suitable GPU device!");
		}
	}

	void Renderer::createLogicalDevice()
	{
		RDE_PROFILE_SCOPE

		QueueFamilyIndices indices = queryQueueFamilies(m_physicalDevice);

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
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.pEnabledFeatures = &deviceFeatures;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(c_deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = c_deviceExtensions.data();

		// No longer needed but added to be compatible with older versions
		if (c_enableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(c_validationLayers.size());
			createInfo.ppEnabledLayerNames = c_validationLayers.data();
		}
		else {
			createInfo.enabledLayerCount = 0;
		}

		if (vkCreateDevice(m_physicalDevice, &createInfo, m_allocator, &m_device) != VK_SUCCESS) {
			RDE_LOG_CRITICAL("Failed to create logical device!");
		}

		// Cache device queue
		vkGetDeviceQueue(m_device, indices.graphicsFamily.value(), 0, &m_graphicsQueue);
		vkGetDeviceQueue(m_device, indices.presentFamily.value(), 0, &m_presentQueue);
	}

	void Renderer::createSwapchain()
	{
		RDE_PROFILE_SCOPE

		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(m_physicalDevice);

		VkSurfaceFormatKHR surfaceFormat = selectSwapSurfaceFormat(swapChainSupport.formats);
		VkPresentModeKHR presentMode = selectSwapPresentMode(swapChainSupport.presentModes);
		VkExtent2D extent = selectSwapExtent(swapChainSupport.capabilities);
		uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

		// If maxImageCount is 0, that means there's no maximum number of images allowed
		// If there's a max, clamp imageCount to the max
		if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
			imageCount = swapChainSupport.capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = m_surface;
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1; // Amount of layers each image consists of
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // Directly render to swapchain

		QueueFamilyIndices indices = queryQueueFamilies(m_physicalDevice);

		if (indices.graphicsFamily != indices.presentFamily) {
			uint32_t indicesArray[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT; // Multiple queue families own image
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = indicesArray;
		}
		else {
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; // One queue family own image exclusively
			createInfo.queueFamilyIndexCount = 0;
			createInfo.pQueueFamilyIndices = nullptr;
		}

		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE; // Clip pixels that are obscured (by another window for example)
		createInfo.oldSwapchain = VK_NULL_HANDLE;

		if (vkCreateSwapchainKHR(m_device, &createInfo, m_allocator, &m_swapchain) != VK_SUCCESS) {
			RDE_LOG_CRITICAL("Failed to create swap chain!");
		}

		// Get current image count after creating swapchain
		vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, nullptr);
		m_swapchainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, m_swapchainImages.data());

		m_swapchainImageFormat = surfaceFormat.format;
		m_swapchainExtent = extent;
	}
	
	void Renderer::createImageViews()
	{
		RDE_PROFILE_SCOPE

		m_swapchainImageViews.resize(m_swapchainImages.size());

		for (size_t i = 0; i < m_swapchainImages.size(); ++i) {
			VkImageViewCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = m_swapchainImages[i];
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = m_swapchainImageFormat;

			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			if (vkCreateImageView(m_device, &createInfo, m_allocator, &m_swapchainImageViews[i]) != VK_SUCCESS) {
				RDE_LOG_CRITICAL("Failed to create image view!");
			}
		}
	}

	void Renderer::createRenderPass()
	{
		RDE_PROFILE_SCOPE

		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = m_swapchainImageFormat;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT; // Single-sampling for now
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // Clear framebuffer to black
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0; // Index
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL; // This refers to implicit subpass BEFORE render pass
		dependency.dstSubpass = 0; // Subpass index, which is 0 since we only have 1 for now
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // Wait for color attachment output stage
		dependency.srcAccessMask = 0;
		// Wait for this stage to finish to allow writing operations
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo renderPassCreateInfo{};
		renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassCreateInfo.attachmentCount = 1;
		renderPassCreateInfo.pAttachments = &colorAttachment;
		renderPassCreateInfo.subpassCount = 1;
		renderPassCreateInfo.pSubpasses = &subpass;
		renderPassCreateInfo.dependencyCount = 1;
		renderPassCreateInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(m_device, &renderPassCreateInfo, m_allocator, &m_renderPass) != VK_SUCCESS) {
			RDE_LOG_CRITICAL("Failed to create render pass!");
		}
	}

	void Renderer::createGraphicsPipeline()
	{
		RDE_PROFILE_SCOPE

		// Shader stage
		auto vertexShaderCode = FileIO::read("assets/shaders/vert.spv");
		auto fragmentShaderCode = FileIO::read("assets/shaders/frag.spv");

		VkShaderModule vertShaderModule = createShaderModule(vertexShaderCode);
		VkShaderModule fragShaderModule = createShaderModule(fragmentShaderCode);

		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShaderModule;
		vertShaderStageInfo.pName = "main"; // Entry point

		VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragShaderModule;
		fragShaderStageInfo.pName = "main"; // Entry point

		std::vector<VkPipelineShaderStageCreateInfo> shaderStages = { vertShaderStageInfo, fragShaderStageInfo };
		
		// Vertex input state
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		// Bindings descriptions are binding spacings between data, per-vertex or per-instance
		vertexInputInfo.vertexBindingDescriptionCount = 0;
		vertexInputInfo.pVertexBindingDescriptions = nullptr;
		// Attributes are passed to vertex shader at a certain binding
		vertexInputInfo.vertexAttributeDescriptionCount = 0;
		vertexInputInfo.pVertexAttributeDescriptions = nullptr;

		// Input assembly state
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
		inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

		// Viewport state
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)m_swapchainExtent.width;
		viewport.height = (float)m_swapchainExtent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = m_swapchainExtent;

		VkPipelineViewportStateCreateInfo viewportStateInfo{};
		viewportStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportStateInfo.viewportCount = 1;
		viewportStateInfo.pViewports = &viewport;
		viewportStateInfo.scissorCount = 1;
		viewportStateInfo.pScissors = &scissor;

		// Rasterization state
		VkPipelineRasterizationStateCreateInfo rasterizerInfo{};
		rasterizerInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizerInfo.depthClampEnable = VK_FALSE; // Clamp fragments within depth range
		rasterizerInfo.rasterizerDiscardEnable = VK_FALSE; // Discard geometry pass through rasterizer
		rasterizerInfo.polygonMode = VK_POLYGON_MODE_FILL; // Fill polygon area with fragments
		rasterizerInfo.lineWidth = 1.0f;
		rasterizerInfo.cullMode = VK_CULL_MODE_BACK_BIT; // Back-face culling
		rasterizerInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterizerInfo.depthBiasEnable = VK_FALSE;
		rasterizerInfo.depthBiasConstantFactor = 0.0f;
		rasterizerInfo.depthBiasClamp = 0.0f;
		rasterizerInfo.depthBiasSlopeFactor = 0.0f;
		
		// Multisampling state
		VkPipelineMultisampleStateCreateInfo multisampleInfo{};
		multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampleInfo.sampleShadingEnable = VK_FALSE;
		multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampleInfo.minSampleShading = 1.0f;
		multisampleInfo.pSampleMask = nullptr;
		multisampleInfo.alphaToCoverageEnable = VK_FALSE;
		multisampleInfo.alphaToOneEnable = VK_FALSE;

		// TODO: Depth and stencil testing state

		// Color blending state
		// Per-framebuffer color blending
		VkPipelineColorBlendAttachmentState colorBlendAttachmentInfo{};
		colorBlendAttachmentInfo.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

#ifdef RDE_ENABLE_ALPHA_BLENDING
		colorBlendAttachmentInfo.blendEnable = VK_TRUE;
		colorBlendAttachmentInfo.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		colorBlendAttachmentInfo.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorBlendAttachmentInfo.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachmentInfo.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachmentInfo.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachmentInfo.alphaBlendOp = VK_BLEND_OP_ADD;
#else
		colorBlendAttachmentInfo.blendEnable = VK_FALSE;
#endif

		// Global color blending
		VkPipelineColorBlendStateCreateInfo colorBlendingInfo{};
		colorBlendingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlendingInfo.logicOpEnable = VK_FALSE; // Setting this to true will void all attachments and use bitwise operations
		colorBlendingInfo.attachmentCount = 1;
		colorBlendingInfo.pAttachments = &colorBlendAttachmentInfo;

		// Dynamic state

		std::vector<VkDynamicState> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_LINE_WIDTH };

		VkPipelineDynamicStateCreateInfo dynamicStateInfo{};
		dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicStateInfo.pDynamicStates = dynamicStates.data();

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pSetLayouts = nullptr;
		pipelineLayoutInfo.pushConstantRangeCount = 0;
		pipelineLayoutInfo.pPushConstantRanges = nullptr;

		if (vkCreatePipelineLayout(m_device, &pipelineLayoutInfo, m_allocator, &m_pipelineLayout) != VK_SUCCESS) {
			RDE_LOG_CRITICAL("Failed to create pipeline layout!");
		}

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		// Shader stages
		pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
		pipelineInfo.pStages = shaderStages.data();
		// Fixed-function states
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;
		pipelineInfo.pViewportState = &viewportStateInfo;
		pipelineInfo.pRasterizationState = &rasterizerInfo;
		pipelineInfo.pMultisampleState = &multisampleInfo;
		pipelineInfo.pDepthStencilState = nullptr;
		pipelineInfo.pColorBlendState = &colorBlendingInfo;
		pipelineInfo.pDynamicState = nullptr;
		// Pipeline layout
		pipelineInfo.layout = m_pipelineLayout;
		// Render pass
		pipelineInfo.renderPass = m_renderPass;
		pipelineInfo.subpass = 0; // Index
		// Pipeline derivatives
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineInfo.basePipelineIndex = -1;

		if (vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &pipelineInfo, m_allocator, &m_graphicsPipeline) != VK_SUCCESS) {
			RDE_LOG_CRITICAL("Failed to create graphics pipeline!");
		}

		vkDestroyShaderModule(m_device, vertShaderModule, m_allocator);
		vkDestroyShaderModule(m_device, fragShaderModule, m_allocator);
	}

	void Renderer::createFramebuffers()
	{
		RDE_PROFILE_SCOPE

		m_swapchainFramebuffers.resize(m_swapchainImageViews.size());

		// Create framebuffer for each image view
		for (size_t i = 0; i < m_swapchainImageViews.size(); ++i) {
			VkImageView attachments[] = {
				m_swapchainImageViews[i]
			};

			VkFramebufferCreateInfo frameBufferInfo{};
			frameBufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			frameBufferInfo.renderPass = m_renderPass;
			frameBufferInfo.attachmentCount = 1;
			frameBufferInfo.pAttachments = attachments;
			frameBufferInfo.width = m_swapchainExtent.width;
			frameBufferInfo.height = m_swapchainExtent.height;
			frameBufferInfo.layers = 1;

			if (vkCreateFramebuffer(m_device, &frameBufferInfo, m_allocator, &m_swapchainFramebuffers[i]) != VK_SUCCESS) {
				RDE_LOG_CRITICAL("Failed to create framebuffer!");
			}
		}
	}
	
	void Renderer::createCommandPool()
	{
		RDE_PROFILE_SCOPE

		QueueFamilyIndices queueFamilyIndices = queryQueueFamilies(m_physicalDevice);

		VkCommandPoolCreateInfo commandPoolInfo{};
		commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		commandPoolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
		commandPoolInfo.flags = 0;

		if (vkCreateCommandPool(m_device, &commandPoolInfo, m_allocator, &m_commandPool) != VK_SUCCESS) {
			RDE_LOG_CRITICAL("Failed to create command pool!");
		}
	}

	void Renderer::createCommandBuffers()
	{
		RDE_PROFILE_SCOPE

		// Create command buffers
		m_commandBuffers.resize(m_swapchainFramebuffers.size());

		VkCommandBufferAllocateInfo allocateInfo{};
		allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocateInfo.commandPool = m_commandPool;
		allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocateInfo.commandBufferCount = static_cast<uint32_t>(m_commandBuffers.size());

		if (vkAllocateCommandBuffers(m_device, &allocateInfo, m_commandBuffers.data()) != VK_SUCCESS) {
			RDE_LOG_CRITICAL("Failed to allocate command buffers!");
		}

		// Record commands
		for (size_t i = 0; i < m_commandBuffers.size(); ++i) {
			// Begin command buffer
			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = 0;
			beginInfo.pInheritanceInfo = nullptr;
			
			if (vkBeginCommandBuffer(m_commandBuffers[i], &beginInfo) != VK_SUCCESS) {
				RDE_LOG_CRITICAL("Failed to begin recording command buffer!");
			}

			{
				// Begin render pass
				VkRenderPassBeginInfo renderPassBeginInfo{};
				renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
				renderPassBeginInfo.renderPass = m_renderPass;
				renderPassBeginInfo.framebuffer = m_swapchainFramebuffers[i];
				renderPassBeginInfo.renderArea.offset = { 0, 0 };
				renderPassBeginInfo.renderArea.extent = m_swapchainExtent;
				// Clear color is 100% black
				renderPassBeginInfo.clearValueCount = 1;
				renderPassBeginInfo.pClearValues = &c_clearColor;

				vkCmdBeginRenderPass(m_commandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

				// Drawing commands
				vkCmdBindPipeline(m_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);
				vkCmdDraw(m_commandBuffers[i], 3, 1, 0, 0); // Draw the triangle

				// End render pass
				vkCmdEndRenderPass(m_commandBuffers[i]);
			}

			if (vkEndCommandBuffer(m_commandBuffers[i]) != VK_SUCCESS) {
				RDE_LOG_CRITICAL("Failed to record command buffer!");
			}
		}

	}

	void Renderer::createSynchronizationObjects()
	{
		RDE_PROFILE_SCOPE

		m_imageAvailableSemaphores.resize(c_maxFramesInFlight);
		m_renderFinishedSemaphores.resize(c_maxFramesInFlight);
		m_inFlightFences.resize(c_maxFramesInFlight);
		m_imagesInFlight.resize(m_swapchainImages.size(), VK_NULL_HANDLE);

		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // Set to signaled state for initial frame

		for (uint32_t i = 0; i < c_maxFramesInFlight; ++i) {

			if (vkCreateSemaphore(m_device, &semaphoreInfo, m_allocator, &m_imageAvailableSemaphores[i]) != VK_SUCCESS ||
				vkCreateSemaphore(m_device, &semaphoreInfo, m_allocator, &m_renderFinishedSemaphores[i]) != VK_SUCCESS ||
				vkCreateFence(m_device, &fenceInfo, m_allocator, &m_inFlightFences[i]) != VK_SUCCESS) {
				RDE_LOG_CRITICAL(fmt::format("Failed to create synchronization objects for frame {}!", i));
			}
		}
	}

	void Renderer::cleanupSwapchain()
	{
		for (auto framebuffer : m_swapchainFramebuffers) {
			vkDestroyFramebuffer(m_device, framebuffer, m_allocator);
		}

		vkFreeCommandBuffers(m_device, m_commandPool, static_cast<uint32_t>(m_commandBuffers.size()), m_commandBuffers.data());

		vkDestroyPipeline(m_device, m_graphicsPipeline, m_allocator);
		vkDestroyPipelineLayout(m_device, m_pipelineLayout, m_allocator);
		vkDestroyRenderPass(m_device, m_renderPass, m_allocator);

		for (auto imageView : m_swapchainImageViews) {
			vkDestroyImageView(m_device, imageView, m_allocator);
		}

		vkDestroySwapchainKHR(m_device, m_swapchain, m_allocator);
	}

	void Renderer::recreateSwapchain()
	{
		// Handle minimization (framebuffer size 0)
		int width = 0, height = 0;
		do {
			glfwGetFramebufferSize(m_window->get(), &width, &height);
			glfwWaitEvents();
		} while (width == 0 || height == 0);

		vkDeviceWaitIdle(m_device);

		cleanupSwapchain();

		createSwapchain();
		createImageViews();
		createRenderPass();
		createGraphicsPipeline();
		createFramebuffers();
		createCommandBuffers();
	}
}
}