#pragma once

#include "utilities/file_parser.hpp"
#include "vulkan/queue_families.hpp"
#include "vulkan/swapchain.hpp"
#include "vulkan/uniform_buffer_object.hpp"
#include "vulkan/vertex.hpp"
#include "window/window.hpp"

#include <vulkan/vulkan.hpp>

namespace RDE {
namespace Vulkan {

	class Renderer
	{
	public:
		void init();
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
		[[nodiscard]] VkFormat retrieveDepthFormat() const;
		[[nodiscard]] VkSampleCountFlagBits retrieveMaxSampleCount() const;
		[[nodiscard]] bool checkGlfwExtensions(const std::vector<VkExtensionProperties>& supportedExtensions, const std::vector<const char*>& glfwExtensions) const;
		[[nodiscard]] bool checkValidationLayerSupport() const;
		[[nodiscard]] bool checkDeviceExtensionSupport(VkPhysicalDevice device) const;
		[[nodiscard]] bool isDeviceSuitable(VkPhysicalDevice device) const;
		[[nodiscard]] bool hasStencilComponent(VkFormat format);
		[[nodiscard]] QueueFamilyIndices queryQueueFamilies(VkPhysicalDevice device) const;
		[[nodiscard]] Swapchain::SupportDetails querySwapchainSupport(VkPhysicalDevice device) const;
		[[nodiscard]] VkSurfaceFormatKHR selectSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) const;
		[[nodiscard]] VkPresentModeKHR selectSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) const;
		[[nodiscard]] VkExtent2D selectSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const;
		[[nodiscard]] uint32_t selectMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;
		[[nodiscard]] VkFormat selectSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const;

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
		void createColorResources();
		void createDepthResources();
		void createTextureImage();
		void createTextureImageView();
		void createTextureSampler();
		void loadModels();
		void createVertexBuffer();
		void createIndexBuffer();
		void createUniformBuffers();
		void createDescriptorPool();
		void createDescriptorSets();
		void createCommandBuffers();
		void createSynchronizationObjects();

		// Resource creation
		[[nodiscard]] VkShaderModule createShaderModule(FileParser::FileBufferType shaderCode) const;
		[[nodiscard]] VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels) const;
		void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkBufferCreateFlags flags, VkMemoryPropertyFlags properties,
			VkBuffer& buffer, VkDeviceMemory& bufferMemory) const;
		void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits sampleCount, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
			VkImage& image, VkDeviceMemory& imageMemory) const;
		void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);

		void cleanupSwapchain();
		void recreateSwapchain();
		void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
		void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);
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

		// Vertices and indices
		std::vector<Vertex> m_vertices;
		std::vector<uint32_t> m_indices;

		// Vertex and Index buffers
		VkBuffer m_vertexBuffer = VK_NULL_HANDLE;
		VkDeviceMemory m_vertexBufferMemory = VK_NULL_HANDLE;
		VkBuffer m_indexBuffer = VK_NULL_HANDLE;
		VkDeviceMemory m_indexBufferMemory = VK_NULL_HANDLE;

		// Uniform and command buffers for each swapchain image
		std::vector<VkCommandBuffer> m_commandBuffers;
		std::vector<VkBuffer> m_uniformBuffers;
		std::vector<VkDeviceMemory> m_uniformBuffersMemory;
		std::vector<VkDescriptorSet> m_descriptorSets;

		// Texture image
		uint32_t m_mipLevels;
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

		// MSAA
		VkSampleCountFlagBits m_msaaSamples = VK_SAMPLE_COUNT_1_BIT;
		VkImage m_colorImage;
		VkDeviceMemory m_colorImageMemory;
		VkImageView m_colorImageView;

		Window* m_window = nullptr;

		size_t m_currentFrame = 0;

		using vertices_value_type = decltype(m_vertices)::value_type;
		using indices_value_type = decltype(m_indices)::value_type;
	};
}
}