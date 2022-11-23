#pragma once

#include "data_types/mesh_instance.hpp"
#include "data_types/pipeline.hpp"
#include "data_types/presentation_mode.hpp"
#include "data_types/push_constant_object.hpp"
#include "data_types/swapchain.hpp"
#include "data_types/vma_buffer.hpp"
#include "data_types/vma_image.hpp"
#include "window/window.hpp"

namespace RDE
{
namespace Vulkan
{
struct Texture;
struct TextureData;
struct QueueFamilyIndices;
struct Vertex;
struct Mesh;

class Renderer
{
  public:
    using InstanceDebugInfo = std::tuple<std::string, std::string, size_t>;

    void init();
    void drawFrame();
    void cleanup();

    void waitForOperations();

    Texture createTextureResources(TextureData& textureData);
    void clearMeshInstances();
    void copyInstancesIntoInstanceBuffer();

    [[nodiscard]] uint32_t drawCallCount() const;
    [[nodiscard]] const std::list<InstanceDebugInfo>& instancesString() const;
    [[nodiscard]] std::vector<MeshInstance>& getInstancesForMesh(uint32_t meshID, uint32_t textureID);

  private:
    // API-specific functions
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                        VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);

    VkResult createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                          const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);

    void destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
                                       const VkAllocationCallbacks* pAllocator);

    // Query functions
    [[nodiscard]] std::vector<VkExtensionProperties> retrieveSupportedExtensionsList() const;
    [[nodiscard]] std::vector<const char*> retrieveRequiredExtensions() const;
    [[nodiscard]] VkFormat retrieveDepthFormat() const;
    [[nodiscard]] VkSampleCountFlagBits retrieveMaxSampleCount() const;
    [[nodiscard]] bool checkGlfwExtensions(const std::vector<VkExtensionProperties>& supportedExtensions,
                                           const std::vector<const char*>& glfwExtensions) const;
    [[nodiscard]] bool checkValidationLayerSupport() const;
    [[nodiscard]] bool checkDeviceExtensionSupport(VkPhysicalDevice device) const;
    [[nodiscard]] bool isDeviceSuitable(VkPhysicalDevice device) const;
    [[nodiscard]] bool hasStencilComponent(VkFormat format);
    [[nodiscard]] bool isMsaaEnabled() const;
    [[nodiscard]] QueueFamilyIndices queryQueueFamilies(VkPhysicalDevice device) const;
    [[nodiscard]] Swapchain::SupportDetails querySwapchainSupport(VkPhysicalDevice device) const;
    [[nodiscard]] VkSurfaceFormatKHR selectSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) const;
    [[nodiscard]] VkPresentModeKHR selectSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) const;
    [[nodiscard]] VkExtent2D selectSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const;
    [[nodiscard]] uint32_t selectMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;
    [[nodiscard]] VkFormat selectSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling,
                                                 VkFormatFeatureFlags features) const;

    // API functions
    void configureDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) const;
    void configureInstanceCreateInfo(VkInstanceCreateInfo& createInfo, const VkApplicationInfo& appInfo,
                                     VkDebugUtilsMessengerCreateInfoEXT& debugMessengerCreateInfo,
                                     const std::vector<const char*>& glfwExtensions) const;

    // Init helpers
    void createInstance();
    void setupDebugMessenger();
    void createSurface();
    void selectPhysicalDevice();
    void createLogicalDevice();
    void createVmaAllocator();
    void createSwapchain();
    void createImageViews();
    void createRenderPass();
    void createDescriptorSetLayout();
    void createFramebuffers();
    void createCommandPools();
    void createColorResources();
    void createDepthResources();
    void loadTextures();
    void loadModels();
    void createVertexBuffers();
    void createIndexBuffers();
    void createUniformBuffers();
    void createDescriptorPool();
    void createDescriptorSets();
    void initImGui();
    void createCommandBuffers();
    void createSynchronizationObjects();

    // Resource creation
    [[nodiscard]] VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels) const;
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags bufferUsage, VkBufferCreateFlags bufferFlags, VmaMemoryUsage allocationUsage,
                      VmaAllocationCreateFlags allocationFlags, VmaBuffer& buffer) const;
    void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits sampleCount, VkFormat format,
                     VkImageTiling tiling, VkImageUsageFlags imageUsage, VmaMemoryUsage allocationUsage,
                     VmaAllocationCreateFlags allocationFlags, VmaImage& vmaImage) const;
    void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);

    // Textures or images
    void createTextureImage(Texture& texture, TextureData& textureData);
    void createTextureImageView(Texture& texture, TextureData& textureData);
    void createTextureSampler(Texture& texture, TextureData& textureData);
    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);

    // Swapchain
    void cleanupSwapchain();
    void recreateSwapchain();

    // Clean up imgui
    void cleanUpImGui();

    // Buffers
    void copyBuffer(const VmaBuffer& srcBuffer, VmaBuffer& dstBuffer, VkDeviceSize size);
    void copyBufferToImage(const VmaBuffer& buffer, VkImage image, uint32_t width, uint32_t height);
    void createVertexBuffer(const std::vector<Vertex>& vertices, VmaBuffer& vertexBuffer);
    void createIndexBuffer(const std::vector<uint32_t>& indices, VmaBuffer& indexBuffer);
    void createInstanceBuffer(InstanceBuffer& instanceBuffer);
    void updateUniformBuffer(uint32_t imageIndex);
    void recordCommandBuffers(uint32_t imageIndex);

    // Commands
    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);
    void drawCommand(VkCommandBuffer commandBuffer, const VkBuffer& vertexBuffer, const VkBuffer& indexBuffer,
                     const InstanceBuffer& instanceBuffer, uint32_t indexCount);

    template <typename TCallable> void singleTimeCommands(TCallable&& callable)
    {
        static_assert(std::is_invocable_v<TCallable, VkCommandBuffer>, "Function needs to take VkCommandBuffer as argument!");

        VkCommandBuffer commandBuffer = beginSingleTimeCommands();
        {
            callable(commandBuffer);
        }
        endSingleTimeCommands(commandBuffer);
    }

    // User-implemented Vulkan objects
    VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;
    VkAllocationCallbacks* m_allocator = VK_NULL_HANDLE;
    VmaAllocator m_vmaAllocator = VK_NULL_HANDLE;

    // Vulkan objects
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    VkInstance m_instance = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;
    VkQueue m_graphicsQueue = VK_NULL_HANDLE;
    VkQueue m_presentQueue = VK_NULL_HANDLE;
    Swapchain m_swapchain;
    VkRenderPass m_renderPass = VK_NULL_HANDLE;
    VkCommandPool m_commandPool = VK_NULL_HANDLE;
    VkCommandPool m_transientCommandPool = VK_NULL_HANDLE;
    Pipeline m_pipeline{};

    // Uniform and command buffers for each swapchain image
    std::vector<VkCommandBuffer> m_commandBuffers;
    std::vector<VmaBuffer> m_uniformBuffers;

    // Descriptor sets
    VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_uboDescriptorSetLayout = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_samplerDescriptorSetLayout = VK_NULL_HANDLE;
    std::vector<VkDescriptorSet> m_uboDescriptorSets;

    // Push constants
    PushConstantObject m_pushConstants;

    // Depth image
    VmaImage m_depthImage{};
    VkImageView m_depthImageView = VK_NULL_HANDLE;

    // Fences and semaphores
    std::vector<VkSemaphore> m_imageAvailableSemaphores;
    std::vector<VkSemaphore> m_renderFinishedSemaphores;
    std::vector<VkFence> m_inFlightFences;
    std::vector<VkFence> m_imagesInFlight;

    // MSAA resources
    VkSampleCountFlagBits m_maxMsaaSamples = VK_SAMPLE_COUNT_1_BIT;
    VmaImage m_colorImage{};
    VkImageView m_colorImageView = VK_NULL_HANDLE;

    // Mesh instances
    std::map<std::pair<uint32_t, uint32_t>, std::unique_ptr<std::vector<Vulkan::MeshInstance>>> m_meshInstances;

    // ImGui vulkan objects
    VkDescriptorPool m_imguiDescriptorPool = VK_NULL_HANDLE;
    Window* m_window = nullptr;

    // Config variables
    // TODO: Create config file to store these values
    VkSampleCountFlagBits m_msaaSamples = VK_SAMPLE_COUNT_8_BIT;
    bool m_enableMipmaps = true;
    uint32_t m_apiVersion = VK_API_VERSION_1_2;
    PresentationMode m_presentationMode = PresentationMode::TripleBuffered;

    // Debugging variables
    size_t m_currentFrame = 0;
    uint32_t m_drawCallCount = 0;
    std::list<InstanceDebugInfo> m_instancesString;
};
} // namespace Vulkan
} // namespace RDE
