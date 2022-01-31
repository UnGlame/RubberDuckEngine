#pragma once

#include "utilities/file_parser/file_parser.hpp"
#include "vulkan/queue_families/queue_families.hpp"
#include "vulkan/swapchain/swapchain.hpp"
#include "vulkan/uniform_buffer_object/uniform_buffer_object.hpp"
#include "vulkan/vertex/vertex.hpp"
#include "window/window.hpp"

#include <vulkan/vulkan.hpp>

namespace RDE {
namespace Vulkan {

	// Constants
	const std::vector<const char*> k_validationLayers = { "VK_LAYER_KHRONOS_validation" };
	const std::vector<const char*> k_deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	const std::vector<Vertex> k_vertices = {
		// Ensure vertices in clockwise order
		Vertex{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
		Vertex{{ 0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
		Vertex{{ 0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
		Vertex{{-0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},

		Vertex{{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
		Vertex{{ 0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
		Vertex{{ 0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
		Vertex{{-0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
	};
	const std::vector<uint16_t> k_indices = {
		0, 1, 2, 2, 3, 0,
		4, 5, 6, 6, 7, 4
	};

	constexpr glm::vec4 k_clearColor = { 0.039f, 0.024f, 0.075f, 1.0f };
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
		[[nodiscard]] VkFormat selectSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const;
		
		[[nodiscard]] inline VkFormat selectDepthFormat() const {
			return selectSupportedFormat(
				{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
				VK_IMAGE_TILING_OPTIMAL,
				VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
		}
		[[nodiscard]] inline bool hasStencilComponent(VkFormat format) { return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT; }

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
		void createDepthResources();
		void createTextureImage();
		void createTextureImageView();
		void createTextureSampler();
		void createVertexBuffer();
		void createIndexBuffer();
		void createUniformBuffers();
		void createDescriptorPool();
		void createDescriptorSets();
		void createCommandBuffers();
		void createSynchronizationObjects();

		// Other utilities
		[[nodiscard]] VkShaderModule createShaderModule(FileParser::FileBufferType shaderCode) const;
		[[nodiscard]] VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) const;

		void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkBufferCreateFlags flags, VkMemoryPropertyFlags properties,
			VkBuffer& buffer, VkDeviceMemory& bufferMemory) const;
		void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
			VkImage& image, VkDeviceMemory& imageMemory) const;

		void cleanupSwapchain();
		void recreateSwapchain();
		void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
		void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
		void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
		void updateUniformBuffer(uint32_t imageIndex);

		// Commands
		VkCommandBuffer beginSingleTimeCommands();
		void endSingleTimeCommands(VkCommandBuffer commandBuffer);
		
		template <typename TCallable>
		void singleTimeCommands(TCallable&& callable)
		{
			static_assert(std::is_invocable_v<TCallable, VkCommandBuffer>, "Commands function is not callable!");

			VkCommandBuffer commandBuffer = beginSingleTimeCommands();
			{
				callable(commandBuffer);
			}
			endSingleTimeCommands(commandBuffer);
		}

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

		// Texture image
		VkImage m_textureImage = VK_NULL_HANDLE;
		VkDeviceMemory m_textureImageMemory = VK_NULL_HANDLE;
		VkImageView m_textureImageView = VK_NULL_HANDLE;
		VkSampler m_textureSampler = VK_NULL_HANDLE;

		// Depth image
		VkImage m_depthImage = VK_NULL_HANDLE;
		VkDeviceMemory m_depthImageMemory = VK_NULL_HANDLE;
		VkImageView m_depthImageView = VK_NULL_HANDLE;

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