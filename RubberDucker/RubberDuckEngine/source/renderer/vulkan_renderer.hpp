#pragma once
#include "utilities/file_io.hpp"

namespace RDE
{
	namespace {
		// Constants
		const std::vector<const char*> c_validationLayers = { "VK_LAYER_KHRONOS_validation" };
		const std::vector<const char*> c_deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
		constexpr VkClearValue c_clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
		constexpr uint32_t c_maxFramesInFlight = 2;
	}

	class VulkanRenderer
	{
	public:
		void init(GLFWwindow* window);
		void drawFrame();
		void cleanup();
		__forceinline void waitForOperations() { vkDeviceWaitIdle(m_device); }
		
	private:
		struct QueueFamilyIndices
		{
			std::optional<uint32_t> graphicsFamily;
			std::optional<uint32_t> presentFamily;

			[[nodiscard]] __forceinline VkBool32 isComplete() const {
				return graphicsFamily && presentFamily;
			}
		};

		struct SwapChainSupportDetails
		{
			VkSurfaceCapabilitiesKHR capabilities;
			std::vector<VkSurfaceFormatKHR> formats;
			std::vector<VkPresentModeKHR> presentModes;
			
			[[nodiscard]] __forceinline VkBool32 isAdequate() const {
				return !formats.empty() && !presentModes.empty();
			}
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
		[[nodiscard]] VkShaderModule createShaderModule(FileIO::FileBufferType shaderCode) const;
		[[nodiscard]] std::vector<VkExtensionProperties> retrieveSupportedExtensionsList() const;
		[[nodiscard]] std::vector<const char*> retrieveRequiredExtensions() const;
		[[nodiscard]] VkBool32 checkGlfwExtensions(const std::vector<VkExtensionProperties>& supportedExtensions, const std::vector<const char*>& glfwExtensions) const;
		[[nodiscard]] VkBool32 checkValidationLayerSupport() const;
		[[nodiscard]] VkBool32 checkDeviceExtensionSupport(VkPhysicalDevice device) const;
		[[nodiscard]] VkBool32 isDeviceSuitable(VkPhysicalDevice device) const;
		[[nodiscard]] QueueFamilyIndices queryQueueFamilies(VkPhysicalDevice device) const;
		[[nodiscard]] SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) const;
		[[nodiscard]] VkSurfaceFormatKHR selectSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) const;
		[[nodiscard]] VkPresentModeKHR selectSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) const;
		[[nodiscard]] VkExtent2D selectSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const;

		void configureDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) const;
		void configureInstanceCreateInfo(VkInstanceCreateInfo& createInfo, const VkApplicationInfo& appInfo, 
			VkDebugUtilsMessengerCreateInfoEXT& debugMessengerCreateInfo, const std::vector<const char*>& glfwExtensions) const;

		// Init helpers
		void createInstance();
		void setupDebugMessenger();
		void createSurface();
		void selectPhysicalDevice();
		void createLogicalDevice();
		void createSwapChain();
		void createImageViews();
		void createRenderPass();
		void createGraphicsPipeline();
		void createFramebuffers();
		void createCommandPool();
		void createCommandBuffers();
		void createSynchronizationObjects();

		// User-implemented Vulkan objects
		VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;
		VkAllocationCallbacks* m_allocator = VK_NULL_HANDLE;

		// Vulkan objects
		VkSurfaceKHR m_surface = VK_NULL_HANDLE;
		VkInstance m_instance = VK_NULL_HANDLE;
		VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
		VkDevice m_device = VK_NULL_HANDLE;
		VkQueue m_graphicsQueue = VK_NULL_HANDLE;
		VkQueue m_presentQueue = VK_NULL_HANDLE;
		VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
		VkRenderPass m_renderPass = VK_NULL_HANDLE;
		VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
		VkPipeline m_graphicsPipeline = VK_NULL_HANDLE;
		VkCommandPool m_commandPool = VK_NULL_HANDLE;

		// Swapchain config
		VkFormat m_swapchainImageFormat;
		VkExtent2D m_swapchainExtent;

		// Swap chain data
		std::vector<VkImage> m_swapchainImages;
		std::vector<VkImageView> m_swapchainImageViews;
		std::vector<VkFramebuffer> m_swapchainFramebuffers;
		std::vector<VkCommandBuffer> m_commandBuffers;

		// Fences and semaphores
		std::vector<VkSemaphore> m_imageAvailableSemaphores;
		std::vector<VkSemaphore> m_renderFinishedSemaphores;
		std::vector<VkFence> m_inFlightFences;
		std::vector<VkFence> m_imagesInFlight;
		size_t m_currentFrame = 0;

		GLFWwindow* m_window = nullptr;

#ifdef RDE_DEBUG
		const bool m_enableValidationLayers = true;
#else
		const bool m_enableValidationLayers = false;
#endif
	};
}