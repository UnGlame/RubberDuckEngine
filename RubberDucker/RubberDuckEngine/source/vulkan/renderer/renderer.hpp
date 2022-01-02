#pragma once

#include "utilities/file_parser/file_parser.hpp"
#include "vulkan/queue_families/queue_families.hpp"
#include "vulkan/swapchain/swapchain.hpp"
#include "vulkan/uniform_buffer_object/uniform_buffer_object.hpp"
#include "vulkan/vertex/vertex.hpp"
#include "window/window.hpp"

namespace RDE {
namespace Vulkan {

	// Constants
	const std::vector<const char*> k_validationLayers = { "VK_LAYER_KHRONOS_validation" };
	const std::vector<const char*> k_deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	const std::array<Vertex, 4> k_vertices = {
		// Ensure vertices in clockwise order
		Vertex{{-0.5f, -0.5f}, {0.937f, 0.278f, 0.435f}},
		Vertex{{ 0.5f, -0.5f}, {1.000f, 0.819f, 0.400f}},
		Vertex{{ 0.5f,  0.5f}, {0.023f, 0.839f, 0.627f}},
		Vertex{{-0.5f,  0.5f}, {0.066f, 0.541f, 0.698f}}
	};
	const std::array<uint16_t, 6> k_indices = {
		0, 1, 2, 2, 3, 0
	};

	constexpr VkClearValue k_clearColor = { {{0.039f, 0.024f, 0.075f, 1.0f}} };
	constexpr uint32_t k_maxFramesInFlight = 3;

#ifdef RDE_DEBUG
	constexpr bool k_enableValidationLayers = true;
#else
	constexpr bool k_enableValidationLayers = false;
#endif

	class Renderer
	{
	public:
		using vertices_value_type = decltype(k_vertices)::value_type;
		using indices_value_type = decltype(k_indices)::value_type;

		void init(Window* window);
		void drawFrame();
		void cleanup();
		inline void waitForOperations() { vkDeviceWaitIdle(m_device); }

	private:
		// API-specific functions
		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData
		);

		VkResult createDebugUtilsMessengerEXT(
			VkInstance instance,
			const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
			const VkAllocationCallbacks* pAllocator,
			VkDebugUtilsMessengerEXT* pDebugMessenger
		);

		void destroyDebugUtilsMessengerEXT(
			VkInstance instance,
			VkDebugUtilsMessengerEXT debugMessenger,
			const VkAllocationCallbacks* pAllocator
		);

		// Query functions
		[[nodiscard]] VkShaderModule createShaderModule(FileParser::FileBufferType shaderCode) const;
		[[nodiscard]] std::vector<VkExtensionProperties> retrieveSupportedExtensionsList() const;
		[[nodiscard]] std::vector<const char*> retrieveRequiredExtensions() const;
		[[nodiscard]] VkBool32 checkGlfwExtensions(const std::vector<VkExtensionProperties>& supportedExtensions, const std::vector<const char*>& glfwExtensions) const;
		[[nodiscard]] VkBool32 checkValidationLayerSupport() const;
		[[nodiscard]] VkBool32 checkDeviceExtensionSupport(VkPhysicalDevice device) const;
		[[nodiscard]] VkBool32 isDeviceSuitable(VkPhysicalDevice device) const;
		[[nodiscard]] QueueFamilyIndices queryQueueFamilies(VkPhysicalDevice device) const;
		[[nodiscard]] Swapchain::SupportDetails querySwapchainSupport(VkPhysicalDevice device) const;
		[[nodiscard]] VkSurfaceFormatKHR selectSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) const;
		[[nodiscard]] VkPresentModeKHR selectSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) const;
		[[nodiscard]] VkExtent2D selectSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const;
		[[nodiscard]] uint32_t selectMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;

		// API functions
		void configureDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) const;
		void configureInstanceCreateInfo(VkInstanceCreateInfo& createInfo, const VkApplicationInfo& appInfo,
			VkDebugUtilsMessengerCreateInfoEXT& debugMessengerCreateInfo, const std::vector<const char*>& glfwExtensions) const;

		// Init helpers
		void createInstance();
		void setupDebugMessenger();
		void createSurface();
		void selectPhysicalDevice();
		void createLogicalDevice();
		void createSwapchain();
		void createImageViews();
		void createRenderPass();
		void createDescriptorSetLayout();
		void createGraphicsPipeline();
		void createFramebuffers();
		void createCommandPools();
		void createVertexBuffer();
		void createIndexBuffer();
		void createUniformBuffers();
		void createDescriptorPool();
		void createDescriptorSets();
		void createCommandBuffers();
		void createSynchronizationObjects();

		// Other utilities
		void cleanupSwapchain();
		void recreateSwapchain();
		void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkBufferCreateFlags flags, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) const;
		void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
		void updateUniformBuffer(uint32_t imageIndex);

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
		Swapchain m_swapchain;
		VkRenderPass m_renderPass = VK_NULL_HANDLE;
		VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
		VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
		VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
		VkPipeline m_graphicsPipeline = VK_NULL_HANDLE;
		VkCommandPool m_commandPool = VK_NULL_HANDLE;
		VkCommandPool m_transientCommandPool = VK_NULL_HANDLE;

		// Buffers
		VkBuffer m_vertexBuffer = VK_NULL_HANDLE;
		VkDeviceMemory m_vertexBufferMemory = VK_NULL_HANDLE;
		VkBuffer m_indexBuffer = VK_NULL_HANDLE;
		VkDeviceMemory m_indexBufferMemory = VK_NULL_HANDLE;

		std::vector<VkCommandBuffer> m_commandBuffers;
		
		std::vector<VkBuffer> m_uniformBuffers;
		std::vector<VkDeviceMemory> m_uniformBuffersMemory;
		std::vector<VkDescriptorSet> m_descriptorSets;

		// Fences and semaphores
		std::vector<VkSemaphore> m_imageAvailableSemaphores;
		std::vector<VkSemaphore> m_renderFinishedSemaphores;
		std::vector<VkFence> m_inFlightFences;
		std::vector<VkFence> m_imagesInFlight;

		Window* m_window = nullptr;

		size_t m_currentFrame = 0;
	};
}
}