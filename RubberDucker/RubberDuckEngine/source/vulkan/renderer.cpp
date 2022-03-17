#include "precompiled/pch.hpp"

#include "utilities/utilities.hpp"
#include "vulkan/renderer.hpp"
#include "vulkan/binding_ids.hpp"

#include "glm/gtc/matrix_transform.hpp"

#include <stbi/stb_image.h>
#include <tinyobjloader/tiny_obj_loader.h>

namespace RDE {
namespace Vulkan {

	// Constants
	const std::vector<const char*> k_validationLayers = { "VK_LAYER_KHRONOS_validation" };
	const std::vector<const char*> k_deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

	const std::string k_modelDirPath = "assets/models/";
	const std::string k_texturePath = "assets/textures/viking_room.png";

	constexpr glm::vec4 k_clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
	constexpr uint32_t k_maxFramesInFlight = 2;

#ifdef RDE_DEBUG
	constexpr bool k_enableValidationLayers = true;
#else
	constexpr bool k_enableValidationLayers = false;
#endif

	void Renderer::init()
	{
		RDE_LOG_INFO("Start");
		m_window = &g_engine->window();

		createInstance();
		setupDebugMessenger();
		createSurface();
		selectPhysicalDevice();
		createLogicalDevice();
		createSwapchain();
		createImageViews();
		createRenderPass();
		createDescriptorSetLayout();
		createGraphicsPipeline();
		createCommandPools();
		createColorResources();
		createDepthResources();
		createFramebuffers();
		loadTextures();
		loadModels();
		createVertexBuffers();
		createIndexBuffers();
		createInstancesMap();
		createInstanceBuffers();
		createUniformBuffers();
		createDescriptorPool();
		createDescriptorSets();
		createCommandBuffers();
		createSynchronizationObjects();
		
		RDE_LOG_INFO("End");
	}

	void Renderer::drawFrame()
	{
		// Log number of draw calls per second
		RDE_LOG_PER_SECOND([&]() {
			RDE_LOG_CLEAN_DEBUG("Number of draw calls currently: {0}", m_nbDrawCalls);
		});

		// Wait for fence at (previous) frame
		vkWaitForFences(m_device, 1, &m_inFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX);

		// Acquire image from swap chain
		uint32_t imageIndex;
		auto result = vkAcquireNextImageKHR(m_device, m_swapchain.handle, UINT64_MAX, m_imageAvailableSemaphores[m_currentFrame], VK_NULL_HANDLE, &imageIndex); // UINT64_MAX disables timeout

		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			recreateSwapchain();
			return;
		}
		RDE_ASSERT_2(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR, "Failed to acquire next swap chain image!");	

		// If previous frame is using this image, we need to wait for its fence
		if (m_imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
			vkWaitForFences(m_device, 1, &m_imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
		}

		// Mark image as being in use by this frame
		m_imagesInFlight[imageIndex] = m_inFlightFences[m_currentFrame];

		//================ Drawing stage ================
		// Update ubo and record command buffer for each model
		m_nbDrawCalls = 0;
		updateUniformBuffer(imageIndex);
		recordCommandBuffers(imageIndex);

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

		result = vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, m_inFlightFences[m_currentFrame]);
		RDE_ASSERT_2(result == VK_SUCCESS, "Failed to submit draw command buffer!");

		// Return image to swap chain for presentation
		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores; // Wait for render to finish
		
		VkSwapchainKHR swapchains[] = { m_swapchain.handle };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapchains;
		presentInfo.pImageIndices = &imageIndex;
		presentInfo.pResults = nullptr;

		result = vkQueuePresentKHR(m_presentQueue, &presentInfo);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_window->isResized()) {
			recreateSwapchain();
			m_window->setResized(false);
		}
		else {
			RDE_ASSERT_2(result == VK_SUCCESS, "Failed to present swapchain image!");
		}

		m_currentFrame = (m_currentFrame + 1) % k_maxFramesInFlight;
	}

	void Renderer::cleanup()
	{
		cleanupSwapchain();

		vkDestroySampler(m_device, m_texture.sampler, m_allocator);
		vkDestroyImageView(m_device, m_texture.imageView, m_allocator);
		vkDestroyImage(m_device, m_texture.image, m_allocator);
		vkFreeMemory(m_device, m_texture.imageMemory, m_allocator);

		vkDestroyDescriptorSetLayout(m_device, m_descriptorSetLayout, m_allocator);

		auto& assetManager = g_engine->assetManager();
		assetManager.eachMesh([this](Mesh& mesh) {
			vkDestroyBuffer(m_device, mesh.instanceBuffer.buffer, m_allocator);
			vkFreeMemory(m_device, mesh.instanceBuffer.memory, m_allocator);

			vkDestroyBuffer(m_device, mesh.instanceBuffer.stagingBuffer, m_allocator);
			vkFreeMemory(m_device, mesh.instanceBuffer.stagingBufferMemory, m_allocator);

			vkDestroyBuffer(m_device, mesh.indexBuffer, m_allocator);
			vkFreeMemory(m_device, mesh.indexBufferMemory, m_allocator);

			vkDestroyBuffer(m_device, mesh.vertexBuffer, m_allocator);
			vkFreeMemory(m_device, mesh.vertexBufferMemory, m_allocator);
		});

		for (uint32_t i = 0; i < k_maxFramesInFlight; ++i) {
			vkDestroySemaphore(m_device, m_imageAvailableSemaphores[i], m_allocator);
			vkDestroySemaphore(m_device, m_renderFinishedSemaphores[i], m_allocator);
			vkDestroyFence(m_device, m_inFlightFences[i], m_allocator);
		}

		vkDestroyCommandPool(m_device, m_commandPool, m_allocator);
		vkDestroyCommandPool(m_device, m_transientCommandPool, m_allocator);

		vkDestroyDevice(m_device, m_allocator);

		if (k_enableValidationLayers) {
			destroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, m_allocator);
		}

		vkDestroySurfaceKHR(m_instance, m_surface, m_allocator);
		vkDestroyInstance(m_instance, nullptr);
	}

	void Renderer::clearMeshInstances() {
		for (auto& [meshID, instances] : m_meshInstances) {
			instances->clear();
		}
	}

	void Renderer::copyInstancesIntoInstanceBuffer()
	{
		static auto& assetManager = g_engine->assetManager();

		for (auto& [meshID, instanceData] : m_meshInstances) {
			if (instanceData->empty()) {
				continue;
			}

			auto& mesh = assetManager.getMesh(meshID);
			auto& instanceBuffer = mesh.instanceBuffer;

			instanceBuffer.instanceCount = static_cast<uint32_t>(instanceData->size());
			uint32_t instanceSize = instanceBuffer.instanceCount * sizeof(Instance);
			
			// If instance count exceeds size, recreate instance buffer with sufficient size
			if (instanceSize > instanceBuffer.size) {
				vkDestroyBuffer(m_device, instanceBuffer.buffer, m_allocator);
				vkFreeMemory(m_device, instanceBuffer.memory, m_allocator);
				vkDestroyBuffer(m_device, instanceBuffer.stagingBuffer, m_allocator);
				vkFreeMemory(m_device, instanceBuffer.stagingBufferMemory, m_allocator);

				// Allocate staging buffer in host visible memory
				createBuffer(
					instanceSize,
					VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
					0,
					VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
					instanceBuffer.stagingBuffer,
					instanceBuffer.stagingBufferMemory
				);

				// Allocate instance buffer in local device memory
				createBuffer(
					instanceSize,
					VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
					0,
					VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
					instanceBuffer.buffer,
					instanceBuffer.memory
				);

				instanceBuffer.size = instanceSize;
			}

			// Fill in host-visible buffer
			void* data;
			vkMapMemory(m_device, instanceBuffer.stagingBufferMemory, 0, instanceSize, 0, &data);
			memcpy(data, instanceData->data(), instanceBuffer.instanceCount * sizeof(Instance));
			vkUnmapMemory(m_device, instanceBuffer.stagingBufferMemory);

			// Copy data from host-visible staging buffer into local device instance buffer using command queue
			copyBuffer(instanceBuffer.stagingBuffer, instanceBuffer.buffer, instanceSize);

			// Set descriptors for instance buffer
			instanceBuffer.descriptor.range = instanceBuffer.size;
			instanceBuffer.descriptor.buffer = instanceBuffer.buffer;
			instanceBuffer.descriptor.offset = 0;
		}
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
	std::vector<VkExtensionProperties> Renderer::retrieveSupportedExtensionsList() const
	{
		uint32_t extensionCount = 0;

		// Retrieve number of supported extensions
		auto result = vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		RDE_ASSERT_0(result == VK_SUCCESS, "Failed to retrieve instance extension count!");

		std::vector<VkExtensionProperties> extensions(extensionCount);

		// Query extension details
		result = vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
		RDE_ASSERT_0(result == VK_SUCCESS, "Failed to retrieve instance extension list!");

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

		if (k_enableValidationLayers) {
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}
		return extensions;
	}

	[[nodiscard]]
	VkFormat Renderer::retrieveDepthFormat() const {
		return selectSupportedFormat(
			{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
	}

	VkSampleCountFlagBits Renderer::retrieveMaxSampleCount() const
	{
		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(m_physicalDevice, &properties);

		VkSampleCountFlags counts = properties.limits.framebufferColorSampleCounts & properties.limits.framebufferDepthSampleCounts;

		return
			counts & VK_SAMPLE_COUNT_64_BIT ?	VK_SAMPLE_COUNT_64_BIT	:
			counts & VK_SAMPLE_COUNT_32_BIT ?	VK_SAMPLE_COUNT_32_BIT	:
			counts & VK_SAMPLE_COUNT_16_BIT ?	VK_SAMPLE_COUNT_16_BIT	:
			counts & VK_SAMPLE_COUNT_8_BIT	?	VK_SAMPLE_COUNT_8_BIT	:
			counts & VK_SAMPLE_COUNT_4_BIT	?	VK_SAMPLE_COUNT_4_BIT	:
			counts & VK_SAMPLE_COUNT_2_BIT	?	VK_SAMPLE_COUNT_2_BIT	:
												VK_SAMPLE_COUNT_1_BIT;
	}

	[[nodiscard]]
	bool Renderer::checkGlfwExtensions(
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
	bool Renderer::checkValidationLayerSupport() const
	{
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (const char* layerName : k_validationLayers) {
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

	bool Renderer::checkDeviceExtensionSupport(VkPhysicalDevice device) const
	{
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		std::unordered_set<std::string> requiredExtensions(k_deviceExtensions.begin(), k_deviceExtensions.end());

		// Eliminate required extension off the checklist for all available ones
		for (const auto& extension : availableExtensions) {
			requiredExtensions.erase(extension.extensionName);
		}

		// If required extensions are empty, it means that available extensions checked them off
		return requiredExtensions.empty();
	}

	[[nodiscard]]
	bool Renderer::isDeviceSuitable(VkPhysicalDevice device) const
	{
		VkPhysicalDeviceProperties deviceProperties;
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceProperties(device, &deviceProperties);
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

		bool isDiscreteGPU = deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
		bool hasRequiredFeatures = deviceFeatures.geometryShader && deviceFeatures.samplerAnisotropy;
		bool hasSuitableQueueFamily = queryQueueFamilies(device).isComplete();
		bool supportsExtensions = checkDeviceExtensionSupport(device);
		bool isSwapchainAdequate;

		if (supportsExtensions) {
			Swapchain::SupportDetails swapchainSupport = querySwapchainSupport(device);
			isSwapchainAdequate = swapchainSupport.isAdequate();
		}

		return isDiscreteGPU && hasRequiredFeatures && hasSuitableQueueFamily && supportsExtensions && isSwapchainAdequate;
	}

	[[nodiscard]]
	bool Renderer::hasStencilComponent(VkFormat format)
	{
		return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
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
			
			auto result = vkGetPhysicalDeviceSurfaceSupportKHR(device, index, m_surface, &presentSupport);
			RDE_ASSERT_0(result == VK_SUCCESS, "Failed to get surface present support!");

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

	Swapchain::SupportDetails Renderer::querySwapchainSupport(VkPhysicalDevice device) const
	{
		Swapchain::SupportDetails details;

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
		glfwGetFramebufferSize(m_window->apiWindow(), &width, &height);

		VkExtent2D actualExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		// Ensure extents are within min and max of capabilities
		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actualExtent;
	}

	uint32_t Renderer::selectMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const
	{
		VkPhysicalDeviceMemoryProperties memoryProperties;
		vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memoryProperties);

		for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i) {
			if (typeFilter & (1 << i) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}

		RDE_ASSERT_0(0, "Failed to find suitable memory type!");
		
		return 0;
	}

	[[nodiscard]]
	VkFormat Renderer::selectSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const
	{
		auto hasFeature = [&](VkFormatFeatureFlags propFeature) -> bool {
			return (propFeature & features) == features;
		};

		for (VkFormat format : candidates) {
			VkFormatProperties properties;
			vkGetPhysicalDeviceFormatProperties(m_physicalDevice, format, &properties);

			if (tiling == VK_IMAGE_TILING_LINEAR && hasFeature(properties.linearTilingFeatures)) {
				return format;
			}
			if (tiling == VK_IMAGE_TILING_OPTIMAL && hasFeature(properties.optimalTilingFeatures)) {
				return format;
			}
			RDE_ASSERT_0(false, "Failed to find supported format from physical device!");
		}

		return VkFormat{};
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

		if (k_enableValidationLayers) {
			bool validationLayersAvailable = checkValidationLayerSupport();
			RDE_ASSERT_0(validationLayersAvailable, "Validation layers requested but not available!");

			createInfo.enabledLayerCount = static_cast<uint32_t>(k_validationLayers.size());
			createInfo.ppEnabledLayerNames = k_validationLayers.data();

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
		auto result = vkCreateInstance(&createInfo, m_allocator, &m_instance);
		RDE_ASSERT_0(result == VK_SUCCESS, "Failed to create Vk instance!");
	
		// Retrieve supported extensions and check against glfw extensions
		const auto supportedExtensions = retrieveSupportedExtensionsList();
	}

	void Renderer::setupDebugMessenger()
	{
		RDE_PROFILE_SCOPE

		if (!k_enableValidationLayers) return;

		VkDebugUtilsMessengerCreateInfoEXT createInfo{};
		configureDebugMessengerCreateInfo(createInfo);

		auto result = createDebugUtilsMessengerEXT(m_instance, &createInfo, m_allocator, &m_debugMessenger);
		RDE_ASSERT_0(result == VK_SUCCESS, "Failed to create debug utils messenger!");
	}

	void Renderer::createSurface()
	{
		RDE_PROFILE_SCOPE

		auto result = glfwCreateWindowSurface(m_instance, m_window->apiWindow(), m_allocator, &m_surface);
		RDE_ASSERT_0(result == VK_SUCCESS, "Failed to create window surface!");
	}

	void Renderer::selectPhysicalDevice()
	{
		RDE_PROFILE_SCOPE

		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);
		
		RDE_ASSERT_0(deviceCount > 0, "Failed to find GPUs with Vulkan support!");

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());

		for (const auto& device : devices) {
			// Select first GPU device to use
			if (isDeviceSuitable(device)) {
				m_physicalDevice = device;
				m_msaaSamples = retrieveMaxSampleCount();
				RDE_LOG_INFO("Number of MSAA Samples: {}", m_msaaSamples);

				break;
			}
		}

		// If at the end variable is still null, no device is suitable
		RDE_ASSERT_0(m_physicalDevice, "Failed to find a suitable GPU device!");
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
		deviceFeatures.samplerAnisotropy = VK_TRUE;

		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.pEnabledFeatures = &deviceFeatures;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(k_deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = k_deviceExtensions.data();

		// No longer needed but added to be compatible with older versions
		if (k_enableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(k_validationLayers.size());
			createInfo.ppEnabledLayerNames = k_validationLayers.data();
		}
		else {
			createInfo.enabledLayerCount = 0;
		}

		auto result = vkCreateDevice(m_physicalDevice, &createInfo, m_allocator, &m_device);
		RDE_ASSERT_0(result == VK_SUCCESS, "Failed to create logical device!");

		// Cache device queue
		vkGetDeviceQueue(m_device, indices.graphicsFamily.value(), 0, &m_graphicsQueue);
		vkGetDeviceQueue(m_device, indices.presentFamily.value(), 0, &m_presentQueue);
	}

	void Renderer::createSwapchain()
	{
		RDE_PROFILE_SCOPE

		Swapchain::SupportDetails swapchainSupport = querySwapchainSupport(m_physicalDevice);

		VkSurfaceFormatKHR surfaceFormat = selectSwapSurfaceFormat(swapchainSupport.formats);
		VkPresentModeKHR presentMode = selectSwapPresentMode(swapchainSupport.presentModes);
		VkExtent2D extent = selectSwapExtent(swapchainSupport.capabilities);
		uint32_t imageCount = swapchainSupport.capabilities.minImageCount + 1;

		// If maxImageCount is 0, that means there's no maximum number of images allowed
		// If there's a max, clamp imageCount to the max
		if (swapchainSupport.capabilities.maxImageCount > 0) {
			imageCount = std::min(imageCount, swapchainSupport.capabilities.maxImageCount);
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

		createInfo.preTransform = swapchainSupport.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE; // Clip pixels that are obscured (by another window for example)
		createInfo.oldSwapchain = VK_NULL_HANDLE;

		auto result = vkCreateSwapchainKHR(m_device, &createInfo, m_allocator, &m_swapchain.handle);
		RDE_ASSERT_0(result == VK_SUCCESS, "Failed to create swap chain!");

		// Get current image count after creating swapchain
		vkGetSwapchainImagesKHR(m_device, m_swapchain.handle, &imageCount, nullptr);
		RDE_LOG_INFO("Number of images/image views/framebuffers/command buffers: {}", imageCount);
		m_swapchain.images.resize(imageCount);
		vkGetSwapchainImagesKHR(m_device, m_swapchain.handle, &imageCount, m_swapchain.images.data());

		m_swapchain.imageFormat = surfaceFormat.format;
		m_swapchain.extent = extent;
	}
	
	void Renderer::createImageViews()
	{
		RDE_PROFILE_SCOPE

		m_swapchain.imageViews.resize(m_swapchain.images.size());

		for (size_t i = 0; i < m_swapchain.images.size(); ++i) {
			m_swapchain.imageViews[i] = createImageView(m_swapchain.images[i], m_swapchain.imageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
		}
	}

	void Renderer::createRenderPass()
	{
		RDE_PROFILE_SCOPE

		// Color attachment
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = m_swapchain.imageFormat;
		colorAttachment.samples = m_msaaSamples;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // Clear framebuffer to black
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0; // Index
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		// Depth stencil attachment
		VkAttachmentDescription depthAttachment{};
		depthAttachment.format = retrieveDepthFormat();
		depthAttachment.samples = m_msaaSamples;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;	// We won't use this attachment after drawing is finished
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; // For stencil tests
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; // For stencil tests
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthAttachmentRef{};
		depthAttachmentRef.attachment = 1; // Index
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		// Color resolve attachment (from MSAA)
		VkAttachmentDescription colorAttachmentResolve{};
		colorAttachmentResolve.format = m_swapchain.imageFormat;
		colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentResolveRef{};
		colorAttachmentResolveRef.attachment = 2; // Index
		colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		// Subpass
		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;
		subpass.pResolveAttachments = &colorAttachmentResolveRef;

		// Dependency
		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL; // This refers to implicit subpass BEFORE render pass
		dependency.dstSubpass = 0; // Subpass index, which is 0 since we only have 1 for now
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT; // Wait for color attachment output and early fragment tests
		dependency.srcAccessMask = 0;

		// Wait for this stage to finish to allow writing operations
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		std::array<VkAttachmentDescription, 3> attachments = { colorAttachment, depthAttachment, colorAttachmentResolve };

		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		auto result = vkCreateRenderPass(m_device, &renderPassInfo, m_allocator, &m_renderPass);
		RDE_ASSERT_0(result == VK_SUCCESS, "Failed to create render pass!");
	}

	void Renderer::createDescriptorSetLayout()
	{
		VkDescriptorSetLayoutBinding uboLayoutBinding{};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		uboLayoutBinding.pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutBinding samplerLayoutBinding{};
		samplerLayoutBinding.binding = 1;
		samplerLayoutBinding.descriptorCount = 1;
		samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerLayoutBinding.pImmutableSamplers = nullptr;
		samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		std::array<VkDescriptorSetLayoutBinding, 2> bindings = { std::move(uboLayoutBinding), std::move(samplerLayoutBinding) };

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutInfo.pBindings = bindings.data();

		auto result = vkCreateDescriptorSetLayout(m_device, &layoutInfo, m_allocator, &m_descriptorSetLayout);
		RDE_ASSERT_0(result == VK_SUCCESS, "Failed to create descriptor set layout!");

	}

	void Renderer::createGraphicsPipeline()
	{
		RDE_PROFILE_SCOPE

		// Shader stage
		auto vertexShaderCode = FileParser::read("assets/shaders/vert.spv");
		auto fragmentShaderCode = FileParser::read("assets/shaders/frag.spv");

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
		std::vector<VkVertexInputBindingDescription> bindingDescriptions = {
			k_bindingDescriptions.getVertexBindingDescription(),
			k_bindingDescriptions.getInstanceBindingDescription()
		};

		std::array<VkVertexInputAttributeDescription, 3> vertexAttrDesc = k_attributeDescriptions.getVertexAttributeDescriptions();
		std::array<VkVertexInputAttributeDescription, 4> instanceAttrDesc = k_attributeDescriptions.getInstanceAttributeDescriptions();
		
		// pos, index, uv
		// transformation matrix columns 0, 1, 2, 3
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions = {
			vertexAttrDesc[0],
			vertexAttrDesc[1],
			vertexAttrDesc[2],

			instanceAttrDesc[0],
			instanceAttrDesc[1],
			instanceAttrDesc[2],
			instanceAttrDesc[3]
		};

		VkPipelineVertexInputStateCreateInfo inputInfo{};
		inputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		
		// Bindings descriptions are binding spacings between data, per-vertex or per-instance
		inputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
		inputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
		
		// Attributes are passed to vertex shader at a certain binding
		inputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		inputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

		// Input assembly state
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
		inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

		// Viewport state
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)m_swapchain.extent.width;
		viewport.height = (float)m_swapchain.extent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = m_swapchain.extent;

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
		rasterizerInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizerInfo.depthBiasEnable = VK_FALSE;
		rasterizerInfo.depthBiasConstantFactor = 0.0f;
		rasterizerInfo.depthBiasClamp = 0.0f;
		rasterizerInfo.depthBiasSlopeFactor = 0.0f;
		
		// Multisampling state
		VkPipelineMultisampleStateCreateInfo multisampleInfo{};
		multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampleInfo.sampleShadingEnable = VK_FALSE;
		multisampleInfo.rasterizationSamples = m_msaaSamples;
		multisampleInfo.minSampleShading = 1.0f;
		multisampleInfo.pSampleMask = nullptr;
		multisampleInfo.alphaToCoverageEnable = VK_FALSE;
		multisampleInfo.alphaToOneEnable = VK_FALSE;

		// Depth and stencil testing state
		VkPipelineDepthStencilStateCreateInfo depthStencilInfo{};
		depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencilInfo.depthTestEnable = VK_TRUE;
		depthStencilInfo.depthWriteEnable = VK_TRUE;
		depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
		depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
		depthStencilInfo.stencilTestEnable = VK_FALSE;

		// Color blending state
		// Per-framebuffer color blending
		VkPipelineColorBlendAttachmentState colorBlendAttachmentInfo{};
		colorBlendAttachmentInfo.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

		// Alpha blending
		colorBlendAttachmentInfo.blendEnable = VK_TRUE;
		colorBlendAttachmentInfo.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		colorBlendAttachmentInfo.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorBlendAttachmentInfo.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachmentInfo.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachmentInfo.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachmentInfo.alphaBlendOp = VK_BLEND_OP_ADD;

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

		// Push Constants
		//VkPushConstantRange pushConstantRange;
		//pushConstantRange.offset = 0;
		//pushConstantRange.size = sizeof(PushConstantObject);
		//pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		// Pipeline layout
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &m_descriptorSetLayout; // UBO descriptor set layout
		//pipelineLayoutInfo.pushConstantRangeCount = 1;
		//pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

		auto result = vkCreatePipelineLayout(m_device, &pipelineLayoutInfo, m_allocator, &m_pipelineLayout);
		RDE_ASSERT_0(result == VK_SUCCESS, "Failed to create pipeline layout!");
		
		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		// Shader stages
		pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
		pipelineInfo.pStages = shaderStages.data();
		// Fixed-function states
		pipelineInfo.pVertexInputState = &inputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;
		pipelineInfo.pViewportState = &viewportStateInfo;
		pipelineInfo.pRasterizationState = &rasterizerInfo;
		pipelineInfo.pMultisampleState = &multisampleInfo;
		pipelineInfo.pDepthStencilState = &depthStencilInfo;
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

		result = vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &pipelineInfo, m_allocator, &m_graphicsPipeline);
		RDE_ASSERT_0(result == VK_SUCCESS, "Failed to create graphics pipeline!");

		vkDestroyShaderModule(m_device, vertShaderModule, m_allocator);
		vkDestroyShaderModule(m_device, fragShaderModule, m_allocator);
	}

	void Renderer::createFramebuffers()
	{
		RDE_PROFILE_SCOPE

		m_swapchain.framebuffers.resize(m_swapchain.imageViews.size());

		// Create framebuffer for each image view
		for (size_t i = 0; i < m_swapchain.imageViews.size(); ++i) {
			std::array<VkImageView, 3> attachments = {
				m_colorImageView,
				m_depthImageView,
				m_swapchain.imageViews[i]
			};

			VkFramebufferCreateInfo frameBufferInfo{};
			frameBufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			frameBufferInfo.renderPass = m_renderPass;
			frameBufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			frameBufferInfo.pAttachments = attachments.data();
			frameBufferInfo.width = m_swapchain.extent.width;
			frameBufferInfo.height = m_swapchain.extent.height;
			frameBufferInfo.layers = 1;

			auto result = vkCreateFramebuffer(m_device, &frameBufferInfo, m_allocator, &m_swapchain.framebuffers[i]);
			RDE_ASSERT_0(result == VK_SUCCESS, "Failed to create framebuffer!");
		}
	}
	
	void Renderer::createCommandPools()
	{
		RDE_PROFILE_SCOPE

		QueueFamilyIndices queueFamilyIndices = queryQueueFamilies(m_physicalDevice);

		VkCommandPoolCreateInfo commandPoolInfo{};
		commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		commandPoolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
		commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		VkCommandPoolCreateInfo transientCommandPoolInfo{};
		transientCommandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		transientCommandPoolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
		transientCommandPoolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

		auto result = vkCreateCommandPool(m_device, &commandPoolInfo, m_allocator, &m_commandPool);
		RDE_ASSERT_0(result == VK_SUCCESS, "Failed to create command pool!");

		result = vkCreateCommandPool(m_device, &transientCommandPoolInfo, m_allocator, &m_transientCommandPool);
		RDE_ASSERT_0(result == VK_SUCCESS, "Failed to create transient command pool!");
	}

	void Renderer::createColorResources()
	{
		VkFormat colorFormat = m_swapchain.imageFormat;

		createImage(m_swapchain.extent.width, m_swapchain.extent.height, 1, m_msaaSamples,
			colorFormat,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			m_colorImage, m_colorImageMemory);

		m_colorImageView = createImageView(m_colorImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
	}

	void Renderer::createDepthResources()
	{
		VkFormat depthFormat = retrieveDepthFormat();
		createImage(m_swapchain.extent.width, m_swapchain.extent.height, 1, m_msaaSamples, depthFormat,
			VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			m_depthImage, m_depthImageMemory);

		m_depthImageView = createImageView(m_depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);

		transitionImageLayout(m_depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1);
	}

	void Renderer::loadTextures()
	{
		createTextureImages();
		createTextureImageViews();
		createTextureSamplers();
	}

	void Renderer::loadModels()
	{
		auto& assetManager = g_engine->assetManager();
		assetManager.loadModels(k_modelDirPath.c_str());
	}

	void Renderer::createVertexBuffers()
	{
		auto& assetManager = g_engine->assetManager();
		assetManager.eachMesh([this](Mesh& mesh) {
			createVertexBuffer(mesh.vertices, mesh.vertexBuffer, mesh.vertexBufferMemory);
		});
	}

	void Renderer::createIndexBuffers()
	{
		auto& assetManager = g_engine->assetManager();
		assetManager.eachMesh([this](Mesh& mesh) {
			createIndexBuffer(mesh.indices, mesh.indexBuffer, mesh.indexBufferMemory);
		});

		assetManager;
	}

	void Renderer::createInstancesMap()
	{
		auto& assetManager = g_engine->assetManager();
		
		assetManager.eachMesh([&](uint32_t meshID, Mesh& mesh) {
			m_meshInstances.insert({ meshID, std::make_unique<std::vector<Instance>>() });
		});
	}

	void Renderer::createVertexBuffer(const std::vector<Vertex>& vertices, VkBuffer& vertexBuffer, VkDeviceMemory& vertexBufferMemory)
	{
		RDE_PROFILE_SCOPE

		// Temporary host-visible buffer
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		VkDeviceSize bufferSize = Utilities::arraysizeof(vertices);

		// size, usage, flags, properties, buffer, bufferMemory
		createBuffer(
			bufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			0,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer,
			stagingBufferMemory
		);

		// Fill in host-visible buffer
		void* data;
		vkMapMemory(m_device, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, vertices.data(), (size_t)bufferSize);
		vkUnmapMemory(m_device, stagingBufferMemory);

		// Allocate vertex buffer in local device memory
		createBuffer(bufferSize,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			0,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			vertexBuffer,
			vertexBufferMemory);

		// Copy data from host-visible staging buffer into local device vertex buffer using command queue
		copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

		// Clean up temporary staging buffer
		vkDestroyBuffer(m_device, stagingBuffer, m_allocator);
		vkFreeMemory(m_device, stagingBufferMemory, m_allocator);
	}

	void Renderer::createIndexBuffer(const std::vector<uint32_t>& indices, VkBuffer& indexBuffer, VkDeviceMemory& indexBufferMemory)
	{
		RDE_PROFILE_SCOPE

		// Temporary host-visible buffer
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		VkDeviceSize bufferSize = Utilities::arraysizeof(indices);

		createBuffer(
			bufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			0,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer,
			stagingBufferMemory);

		// Fill in host-visible buffer
		void* data;
		vkMapMemory(m_device, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, indices.data(), (size_t)bufferSize);
		vkUnmapMemory(m_device, stagingBufferMemory);

		// Allocate index buffer in local device memory
		createBuffer(
			bufferSize,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			0,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			indexBuffer,
			indexBufferMemory
		);

		// Copy data from host-visible staging buffer into local device index buffer using command queue
		copyBuffer(stagingBuffer, indexBuffer, bufferSize);
		
		// Clean up temporary staging buffer
		vkDestroyBuffer(m_device, stagingBuffer, m_allocator);
		vkFreeMemory(m_device, stagingBufferMemory, m_allocator);
	}

	void Renderer::createInstanceBuffer(InstanceBuffer& instanceBuffer)
	{
		// Default to fit 512 instances first
		instanceBuffer.size = 512 * sizeof(Instance);

		// Allocate staging buffer in host visible memory
		createBuffer(
			instanceBuffer.size,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			0,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			instanceBuffer.stagingBuffer,
			instanceBuffer.stagingBufferMemory
		);
		createBuffer(
			instanceBuffer.size,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			0,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			instanceBuffer.buffer,
			instanceBuffer.memory
		);
	}

	void Renderer::createUniformBuffers()
	{
		RDE_PROFILE_SCOPE

		VkDeviceSize bufferSize = sizeof(UniformBufferObject);
		
		m_uniformBuffers.resize(m_swapchain.images.size());
		m_uniformBuffersMemory.resize(m_swapchain.images.size());

		for (size_t i = 0; i < m_swapchain.images.size(); ++i) {
			createBuffer(
				bufferSize,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				0,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				m_uniformBuffers[i],
				m_uniformBuffersMemory[i]
			);
		}
	}

	void Renderer::createInstanceBuffers()
	{
		RDE_PROFILE_SCOPE

		auto& assetManager = g_engine->assetManager();
		assetManager.eachMesh([this](uint32_t meshID, Mesh& mesh) {
			auto& instanceBuffer = mesh.instanceBuffer;
			createInstanceBuffer(instanceBuffer);
		});
	}

	void Renderer::createDescriptorPool()
	{
		RDE_PROFILE_SCOPE

		VkDescriptorPoolSize uboPoolSize{};
		uboPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboPoolSize.descriptorCount = static_cast<uint32_t>(m_swapchain.images.size());

		VkDescriptorPoolSize samplerPoolSize{};
		samplerPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerPoolSize.descriptorCount = static_cast<uint32_t>(m_swapchain.images.size());

		std::array<VkDescriptorPoolSize, 2> poolSizes = { std::move(uboPoolSize), std::move(samplerPoolSize) };

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = static_cast<uint32_t>(m_swapchain.images.size());
		poolInfo.flags = 0;

		auto result = vkCreateDescriptorPool(m_device, &poolInfo, m_allocator, &m_descriptorPool);
		RDE_ASSERT_0(result == VK_SUCCESS, "Failed to create descriptor pool!");
	}

	void Renderer::createDescriptorSets()
	{
		RDE_PROFILE_SCOPE

		// Create vector of swapchain images number of same layouts
		std::vector<VkDescriptorSetLayout> layouts(m_swapchain.images.size(), m_descriptorSetLayout);

		VkDescriptorSetAllocateInfo allocateInfo{};
		allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocateInfo.descriptorPool = m_descriptorPool;
		allocateInfo.descriptorSetCount = static_cast<uint32_t>(m_swapchain.images.size());
		allocateInfo.pSetLayouts = layouts.data();

		m_descriptorSets.resize(m_swapchain.images.size());

		auto result = vkAllocateDescriptorSets(m_device, &allocateInfo, m_descriptorSets.data());
		RDE_ASSERT_2(result == VK_SUCCESS, "Failed to allocate descriptor sets!");

		for (size_t i = 0; i < m_swapchain.images.size(); ++i) {
			// For uniform buffer
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = m_uniformBuffers[i];
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(UniformBufferObject);

			VkWriteDescriptorSet uboDescriptorWrite{};
			uboDescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			uboDescriptorWrite.dstSet = m_descriptorSets[i];
			uboDescriptorWrite.dstBinding = 0;
			uboDescriptorWrite.dstArrayElement = 0;
			uboDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			uboDescriptorWrite.descriptorCount = 1;
			uboDescriptorWrite.pBufferInfo = &bufferInfo;
			uboDescriptorWrite.pImageInfo = nullptr;
			uboDescriptorWrite.pTexelBufferView = nullptr;

			// For image sampler
			VkDescriptorImageInfo imageInfo{};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = m_texture.imageView;
			imageInfo.sampler = m_texture.sampler;

			VkWriteDescriptorSet samplerDescriptorWrite{};
			samplerDescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			samplerDescriptorWrite.dstSet = m_descriptorSets[i];
			samplerDescriptorWrite.dstBinding = 1;
			samplerDescriptorWrite.dstArrayElement = 0;
			samplerDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			samplerDescriptorWrite.descriptorCount = 1;
			samplerDescriptorWrite.pBufferInfo = nullptr;
			samplerDescriptorWrite.pImageInfo = &imageInfo;
			samplerDescriptorWrite.pTexelBufferView = nullptr;

			std::array<VkWriteDescriptorSet, 2> descriptorWrites = { std::move(uboDescriptorWrite), std::move(samplerDescriptorWrite) };

			vkUpdateDescriptorSets(m_device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
		}
	}

	void Renderer::createCommandBuffers()
	{
		RDE_PROFILE_SCOPE

		// Create command buffers
		m_commandBuffers.resize(m_swapchain.framebuffers.size());

		VkCommandBufferAllocateInfo allocateInfo{};
		allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocateInfo.commandPool = m_commandPool;
		allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocateInfo.commandBufferCount = static_cast<uint32_t>(m_commandBuffers.size());

		auto result = vkAllocateCommandBuffers(m_device, &allocateInfo, m_commandBuffers.data());
		RDE_ASSERT_0(result == VK_SUCCESS, "Failed to allocate command buffers!");

		for (size_t imageIndex = 0; imageIndex < m_swapchain.images.size(); ++imageIndex) {
			recordCommandBuffers(static_cast<uint32_t>(imageIndex));
		}
	}

	void Renderer::createSynchronizationObjects()
	{
		RDE_PROFILE_SCOPE

		m_imageAvailableSemaphores.resize(k_maxFramesInFlight);
		m_renderFinishedSemaphores.resize(k_maxFramesInFlight);
		m_inFlightFences.resize(k_maxFramesInFlight);
		m_imagesInFlight.resize(m_swapchain.images.size(), VK_NULL_HANDLE);

		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // Set to signaled state for initial frame

		for (uint32_t i = 0; i < k_maxFramesInFlight; ++i) {

			auto success =
				vkCreateSemaphore(m_device, &semaphoreInfo, m_allocator, &m_imageAvailableSemaphores[i])	== VK_SUCCESS &&
				vkCreateSemaphore(m_device, &semaphoreInfo, m_allocator, &m_renderFinishedSemaphores[i])	== VK_SUCCESS &&
				vkCreateFence(m_device, &fenceInfo, m_allocator, &m_inFlightFences[i])						== VK_SUCCESS;

			RDE_ASSERT_0(success, "Failed to create synchronization objects for frame {}!", i);
		}
	}

	[[nodiscard]]
	VkShaderModule Renderer::createShaderModule(FileParser::FileBufferType shaderCode) const
	{
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = shaderCode.size();
		// Each 4 chars of code is stored as 1 uint32_t
		createInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode.data());

		VkShaderModule shaderModule;
		auto result = vkCreateShaderModule(m_device, &createInfo, m_allocator, &shaderModule);
		RDE_ASSERT_0(result == VK_SUCCESS, "Failed to create shader module!");

		return shaderModule;
	}

	[[nodiscard]]
	VkImageView Renderer::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels) const
	{
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = aspectFlags;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = mipLevels;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		VkImageView imageView;

		RDE_ASSERT_0(vkCreateImageView(m_device, &viewInfo, m_allocator, &imageView) == VK_SUCCESS, "Failed to create image view!");

		return imageView;
	}

	void Renderer::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkBufferCreateFlags flags, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) const
	{
		// Create buffer
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // Only allow one queue to use
		bufferInfo.flags = flags;

		auto result = vkCreateBuffer(m_device, &bufferInfo, m_allocator, &buffer);
		RDE_ASSERT_0(result == VK_SUCCESS, "Failed to create buffer!");

		// Allocate buffer memory
		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(m_device, buffer, &memRequirements);

		VkMemoryAllocateInfo allocateInfo{};
		allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocateInfo.allocationSize = memRequirements.size;
		allocateInfo.memoryTypeIndex = selectMemoryType(memRequirements.memoryTypeBits, properties);

		result = vkAllocateMemory(m_device, &allocateInfo, m_allocator, &bufferMemory);
		RDE_ASSERT_0(result == VK_SUCCESS, "Failed to allocate memory for buffer!");

		// Bind buffer to buffer memory
		vkBindBufferMemory(m_device, buffer, bufferMemory, 0);
	}

	void Renderer::createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits sampleCount, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) const
	{
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = static_cast<uint32_t>(width);
		imageInfo.extent.height = static_cast<uint32_t>(height);
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = mipLevels;
		imageInfo.arrayLayers = 1;
		imageInfo.format = format;
		imageInfo.tiling = tiling;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = usage;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.samples = sampleCount;
		imageInfo.flags = 0;

		RDE_ASSERT_0(vkCreateImage(m_device, &imageInfo, m_allocator, &image) == VK_SUCCESS, "Failed to create image!");

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(m_device, image, &memRequirements);

		VkMemoryAllocateInfo allocateInfo{};
		allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocateInfo.allocationSize = memRequirements.size;
		allocateInfo.memoryTypeIndex = selectMemoryType(memRequirements.memoryTypeBits, properties);

		RDE_ASSERT_0(vkAllocateMemory(m_device, &allocateInfo, m_allocator, &imageMemory) == VK_SUCCESS, "Failed to allocate image memory!");

		vkBindImageMemory(m_device, image, imageMemory, 0);
	}

	void Renderer::generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels)
	{
		// Check if image format supports linear blitting
		VkFormatProperties formatProperties;
		vkGetPhysicalDeviceFormatProperties(m_physicalDevice, imageFormat, &formatProperties);

		RDE_ASSERT_0(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT, "Texture image format does not support linear blitting!");

		singleTimeCommands([&](VkCommandBuffer commandBuffer) {
			VkImageMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.image = image;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;
			barrier.subresourceRange.levelCount = 1;

			int32_t mipWidth = texWidth;
			int32_t mipHeight = texHeight;
			
			// Blit image at each mip level
			if (m_enableMipmaps) {
				for (uint32_t i = 1; i < mipLevels; ++i) {
					barrier.subresourceRange.baseMipLevel = i - 1;
					barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
					barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
					barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
					barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

					// Wait for image layout transition
					vkCmdPipelineBarrier(
						commandBuffer,
						VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
						0, nullptr,
						0, nullptr,
						1, &barrier);

					VkImageBlit blit{};
					blit.srcOffsets[0] = { 0, 0, 0 };
					blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
					blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
					blit.srcSubresource.mipLevel = i - 1;
					blit.srcSubresource.baseArrayLayer = 0;
					blit.srcSubresource.layerCount = 1;
					blit.dstOffsets[0] = { 0, 0, 0 };
					blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth >> 1 : 1, mipHeight > 1 ? mipHeight >> 1 : 1, 1 };
					blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
					blit.dstSubresource.mipLevel = i;
					blit.dstSubresource.baseArrayLayer = 0;
					blit.dstSubresource.layerCount = 1;

					// Blit the image
					vkCmdBlitImage(
						commandBuffer,
						image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
						image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
						1, &blit,
						VK_FILTER_LINEAR);

					barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
					barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
					barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
					barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

					// Wait for layout transition
					vkCmdPipelineBarrier(
						commandBuffer,
						VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
						0, nullptr,
						0, nullptr,
						1, &barrier);

					if (mipWidth > 1) mipWidth /= 2;
					if (mipHeight > 1) mipHeight /= 2;
				}
			}
			barrier.subresourceRange.baseMipLevel = mipLevels - 1;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			// Wait for final mip level
			vkCmdPipelineBarrier(
				commandBuffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
				0, nullptr,
				0, nullptr,
				1, &barrier);
		});
	}

	void Renderer::createTextureImages()
	{
		RDE_PROFILE_SCOPE

		static auto& assetManager = g_engine->assetManager();

		int texWidth, texHeight, texChannels;
		
		// Load texture
		stbi_uc* pixels = assetManager.loadTexture(k_texturePath.c_str(), texWidth, texHeight, texChannels);
		VkDeviceSize imageSize = texWidth * texHeight * STBI_rgb_alpha;

		// Set miplevels
		m_texture.mipLevels = m_enableMipmaps ? static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1 : 1;

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 0, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(m_device, stagingBufferMemory, 0, imageSize, 0, &data);
		memcpy(data, pixels, static_cast<size_t>(imageSize));
		vkUnmapMemory(m_device, stagingBufferMemory);

		assetManager.freeTexture(pixels);

		createImage(texWidth, texHeight, m_texture.mipLevels, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_texture.image, m_texture.imageMemory);

		transitionImageLayout(m_texture.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, m_texture.mipLevels);
		copyBufferToImage(stagingBuffer, m_texture.image, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
		generateMipmaps(m_texture.image, VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight, m_texture.mipLevels);

		vkDestroyBuffer(m_device, stagingBuffer, m_allocator);
		vkFreeMemory(m_device, stagingBufferMemory, m_allocator);
	}

	void Renderer::createTextureImageViews()
	{
		RDE_PROFILE_SCOPE

		m_texture.imageView = createImageView(m_texture.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, m_texture.mipLevels);
	}

	void Renderer::createTextureSamplers()
	{
		RDE_PROFILE_SCOPE

			VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.anisotropyEnable = VK_TRUE;

		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(m_physicalDevice, &properties);
		samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = m_enableMipmaps ? VK_SAMPLER_MIPMAP_MODE_LINEAR : VK_SAMPLER_MIPMAP_MODE_NEAREST;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = m_enableMipmaps ? static_cast<float>(m_texture.mipLevels) : VK_LOD_CLAMP_NONE;

		RDE_ASSERT_0(vkCreateSampler(m_device, &samplerInfo, m_allocator, &m_texture.sampler) == VK_SUCCESS, "Failed to create texture sampler!");
	}

	void Renderer::cleanupSwapchain()
	{
		vkDestroyImageView(m_device, m_colorImageView, m_allocator);
		vkDestroyImage(m_device, m_colorImage, m_allocator);
		vkFreeMemory(m_device, m_colorImageMemory, m_allocator);

		vkDestroyImageView(m_device, m_depthImageView, m_allocator);
		vkDestroyImage(m_device, m_depthImage, m_allocator);
		vkFreeMemory(m_device, m_depthImageMemory, m_allocator);

		for (auto framebuffer : m_swapchain.framebuffers) {
			vkDestroyFramebuffer(m_device, framebuffer, m_allocator);
		}

		vkFreeCommandBuffers(m_device, m_commandPool, static_cast<uint32_t>(m_commandBuffers.size()), m_commandBuffers.data());

		vkDestroyPipeline(m_device, m_graphicsPipeline, m_allocator);
		vkDestroyPipelineLayout(m_device, m_pipelineLayout, m_allocator);
		vkDestroyRenderPass(m_device, m_renderPass, m_allocator);

		for (auto imageView : m_swapchain.imageViews) {
			vkDestroyImageView(m_device, imageView, m_allocator);
		}

		vkDestroySwapchainKHR(m_device, m_swapchain.handle, m_allocator);

		for (size_t i = 0; i < m_swapchain.images.size(); ++i) {
			vkDestroyBuffer(m_device, m_uniformBuffers[i], m_allocator);
			vkFreeMemory(m_device, m_uniformBuffersMemory[i], m_allocator);
		}

		vkDestroyDescriptorPool(m_device, m_descriptorPool, m_allocator);
	}

	void Renderer::recreateSwapchain()
	{
		RDE_PROFILE_SCOPE

		// Handle minimization (framebuffer size 0)
		int width = 0, height = 0;
		do {
			glfwGetFramebufferSize(m_window->apiWindow(), &width, &height);
			glfwWaitEvents();
		} while (width == 0 || height == 0);

		vkDeviceWaitIdle(m_device);

		cleanupSwapchain();

		createSwapchain();
		createImageViews();
		createRenderPass();
		createGraphicsPipeline();
		createColorResources();
		createDepthResources();
		createFramebuffers();
		createUniformBuffers();
		createDescriptorPool();
		createDescriptorSets();
		createCommandBuffers();
	}

	void Renderer::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
	{
		singleTimeCommands([&](VkCommandBuffer commandBuffer) {
			VkBufferCopy copyRegion{};
			copyRegion.srcOffset = 0;
			copyRegion.dstOffset = 0;
			copyRegion.size = size;
			vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
		});
	}

	void Renderer::updateUniformBuffer(uint32_t imageIndex)
	{
		UniformBufferObject ubo{};

		const auto& camera = g_engine->scene().camera();

		ubo.view = glm::lookAt(camera.eye, camera.eye + camera.front, camera.up);
		ubo.projection = glm::perspective(glm::radians(camera.fov), m_swapchain.extent.width / (float)m_swapchain.extent.height, camera.nearClip, camera.farClip);

		// Flip Y
		ubo.projection[1][1] *= -1.0f;

		void* data;
		vkMapMemory(m_device, m_uniformBuffersMemory[imageIndex], 0, sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(m_device, m_uniformBuffersMemory[imageIndex]);
	}

	void Renderer::recordCommandBuffers(uint32_t imageIndex)
	{
		VkResult result;

		// Explicitly reset command buffer to initial state
		RDE_ASSERT_2(vkResetCommandBuffer(m_commandBuffers[imageIndex], VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT) == VK_SUCCESS, "Failed to reset command buffer!");

		// Record commands
		// Begin command buffer
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0;
		beginInfo.pInheritanceInfo = nullptr;

		result = vkBeginCommandBuffer(m_commandBuffers[imageIndex], &beginInfo);
		RDE_ASSERT_0(result == VK_SUCCESS, "Failed to begin recording command buffer!");
		{
			VkRenderPassBeginInfo renderPassBeginInfo{};
			renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassBeginInfo.renderPass = m_renderPass;
			renderPassBeginInfo.framebuffer = m_swapchain.framebuffers[imageIndex];
			renderPassBeginInfo.renderArea.offset = { 0, 0 };
			renderPassBeginInfo.renderArea.extent = m_swapchain.extent;

			std::array<VkClearValue, 2> clearValues{};
			clearValues[0].color = { k_clearColor.x, k_clearColor.y, k_clearColor.z, k_clearColor.w };
			clearValues[1].depthStencil = { 1.0f, 0 };

			renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
			renderPassBeginInfo.pClearValues = clearValues.data();

			vkCmdBeginRenderPass(m_commandBuffers[imageIndex], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

			// Bind graphics pipeline
			vkCmdBindPipeline(m_commandBuffers[imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);

			// Bind descriptor sets (First descriptor set only for now) TODO: Have 1 descriptor set for each model to support multiple textures/materials
			vkCmdBindDescriptorSets(m_commandBuffers[imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, m_descriptorSets.data(), 0, nullptr);

			// Draw each object with model component
			auto group = g_engine->registry().group<TransformComponent, ModelComponent>();
			static auto& assetManager = g_engine->assetManager();

			assetManager.eachMesh([&](uint32_t meshID, Mesh& mesh) {
				drawCommand(m_commandBuffers[imageIndex], meshID, mesh);
			});
			vkCmdEndRenderPass(m_commandBuffers[imageIndex]);
		}
		result = vkEndCommandBuffer(m_commandBuffers[imageIndex]);
		RDE_ASSERT_0(result == VK_SUCCESS, "Failed to record command buffer!");
	}

	void Renderer::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels)
	{
		singleTimeCommands([&](VkCommandBuffer commandBuffer) {
			VkImageMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.oldLayout = oldLayout;
			barrier.newLayout = newLayout;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = image;

			if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
				barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

				if (hasStencilComponent(format)) {
					barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
				}
			}
			else {
				barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			}

			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = mipLevels;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;

			VkPipelineStageFlags sourceStage{}, destinationStage{};

			// Only allow destination write when layouts are correct
			if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
				barrier.srcAccessMask = 0;
				barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

				sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
				destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			}
			else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

				sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
				destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			}
			else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
				barrier.srcAccessMask = 0;
				barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

				sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
				destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			}
			else {
				RDE_ASSERT_0(false, "Unsupported layout transition used!");
			}

			vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
		});
	}

	void Renderer::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
	{
		singleTimeCommands([&](VkCommandBuffer commandBuffer) {
			VkBufferImageCopy region{};
			region.bufferOffset = 0;
			region.bufferRowLength = 0;
			region.bufferImageHeight = 0;

			region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			region.imageSubresource.mipLevel = 0;
			region.imageSubresource.baseArrayLayer = 0;
			region.imageSubresource.layerCount = 1;

			region.imageOffset = { 0, 0, 0 };
			region.imageExtent = { width, height , 1 };

			vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
		});
	}

	VkCommandBuffer Renderer::beginSingleTimeCommands()
	{
		VkCommandBufferAllocateInfo allocateInfo{};
		allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocateInfo.commandPool = m_transientCommandPool; // Use transientCommandPool for short-lived command buffers
		allocateInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(m_device, &allocateInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		return commandBuffer;
	}

	void Renderer::endSingleTimeCommands(VkCommandBuffer commandBuffer)
	{
		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(m_graphicsQueue);

		vkFreeCommandBuffers(m_device, m_transientCommandPool, 1, &commandBuffer);
	}

	void Renderer::drawCommand(VkCommandBuffer commandBuffer, uint32_t meshID, const Mesh& mesh)
	{
		if (!mesh.instanceBuffer.instanceCount) {
			return;
		}

		// TODO: Batch all VBs and IBs into one and use indexing)
		VkDeviceSize offsets[] = { 0 };

		vkCmdBindVertexBuffers(commandBuffer, VertexBufferBindingID, 1, &mesh.vertexBuffer, offsets);
		vkCmdBindVertexBuffers(commandBuffer, InstanceBufferBindingID, 1, &mesh.instanceBuffer.buffer, offsets);

		static_assert(std::is_same_v<Mesh::IndicesValueType, uint16_t> || std::is_same_v<Mesh::IndicesValueType, uint32_t>,
			"Index buffer is not uint32_t or uint16_t!");

		// Bind index buffer for this mesh
		if constexpr (std::is_same_v<Mesh::IndicesValueType, std::uint16_t>) {
			vkCmdBindIndexBuffer(commandBuffer, mesh.indexBuffer, 0, VK_INDEX_TYPE_UINT16);
		}
		else {
			vkCmdBindIndexBuffer(commandBuffer, mesh.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
		}

		//vkCmdPushConstants(commandBuffer, m_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &m_pushConstants.modelMtx);

		// Draw command for this mesh
		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(mesh.indices.size()), mesh.instanceBuffer.instanceCount, 0, 0, 0);
		++m_nbDrawCalls;

	}
}
}