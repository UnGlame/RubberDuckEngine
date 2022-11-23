﻿#include "precompiled/pch.hpp"

#include "utilities/utilities.hpp"
#include "vulkan/binding_ids.hpp"
#include "vulkan/renderer.hpp"

#include "glm/gtc/matrix_transform.hpp"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include <stbi/stb_image.h>
#include <tinyobjloader/tiny_obj_loader.h>

#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>

namespace RDE
{
namespace Vulkan
{

// Constants
const std::vector<const char*> k_validationLayers = {"VK_LAYER_KHRONOS_validation"};
const std::vector<const char*> k_deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

const std::string k_modelDirPath = "assets/models/";
const std::string k_textureDirPath = "assets/textures/";

const glm::vec4 k_clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
constexpr uint32_t k_maxFramesInFlight = 2;

#ifdef RDE_ENABLE_VALIDATION_LAYERS
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
    createVmaAllocator();
    createSwapchain();
    createImageViews();
    createRenderPass();
    createDescriptorSetLayout();
    m_pipeline.create(m_device, m_allocator, m_swapchain, m_msaaSamples, m_uboDescriptorSetLayout, m_samplerDescriptorSetLayout,
                      m_renderPass);
    createCommandPools();
    createColorResources();
    createDepthResources();
    createFramebuffers();
    loadTextures();
    loadModels();
    createVertexBuffers();
    createIndexBuffers();
    createUniformBuffers();
    createDescriptorPool();
    createDescriptorSets();
    initImGui();
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
    auto result =
        vkAcquireNextImageKHR(m_device, m_swapchain.handle, UINT64_MAX, m_imageAvailableSemaphores[m_currentFrame], VK_NULL_HANDLE,
                              &imageIndex); // UINT64_MAX disables timeout

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

    // Render ImGui
    if (g_engine->editor().renderingEnabled()) {
        ImGui::Render();
    }
    // Update ubo and record command buffer for each model
    m_drawCallCount = 0;
    updateUniformBuffer(imageIndex);
    recordCommandBuffers(imageIndex);

    // Execute command buffer with image as attachment in the framebuffer
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {m_imageAvailableSemaphores[m_currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT}; // Each stage
                                                                                         // corresponds to each
                                                                                         // wait semaphore
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_commandBuffers[imageIndex];

    VkSemaphore signalSemaphores[] = {m_renderFinishedSemaphores[m_currentFrame]};
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

    VkSwapchainKHR swapchains[] = {m_swapchain.handle};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapchains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr;

    result = vkQueuePresentKHR(m_presentQueue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_window->isResized()) {
        recreateSwapchain();
        m_window->setResized(false);
    } else {
        RDE_ASSERT_2(result == VK_SUCCESS, "Failed to present swapchain image!");
    }

    m_currentFrame = (m_currentFrame + 1) % k_maxFramesInFlight;
}

void Renderer::cleanup()
{
    cleanupSwapchain();
    cleanUpImGui();

    auto& assetManager = g_engine->assetManager();
    assetManager.eachTexture([this](Texture& texture) {
        vkDestroySampler(m_device, texture.sampler, m_allocator);
        vkDestroyImageView(m_device, texture.imageView, m_allocator);
        vkDestroyImage(m_device, texture.image, m_allocator);
        vkFreeMemory(m_device, texture.imageMemory, m_allocator);
    });

    vkDestroyDescriptorSetLayout(m_device, m_samplerDescriptorSetLayout, m_allocator);
    vkDestroyDescriptorSetLayout(m_device, m_uboDescriptorSetLayout, m_allocator);

    assetManager.eachMesh([this](Mesh& mesh) {
        for (auto [textureID, instanceBuffer] : mesh.instanceBuffers) {
            vmaDestroyBuffer(m_vmaAllocator, instanceBuffer.vmaBuffer.buffer, instanceBuffer.vmaBuffer.allocation);
            vmaDestroyBuffer(m_vmaAllocator, instanceBuffer.stagingBuffer.buffer, instanceBuffer.stagingBuffer.allocation);
        }
        vmaDestroyBuffer(m_vmaAllocator, mesh.indexBuffer.buffer, mesh.indexBuffer.allocation);
        vmaDestroyBuffer(m_vmaAllocator, mesh.vertexBuffer.buffer, mesh.vertexBuffer.allocation);
    });

    for (uint32_t i = 0; i < k_maxFramesInFlight; ++i) {
        vkDestroySemaphore(m_device, m_imageAvailableSemaphores[i], m_allocator);
        vkDestroySemaphore(m_device, m_renderFinishedSemaphores[i], m_allocator);
        vkDestroyFence(m_device, m_inFlightFences[i], m_allocator);
    }

    vkDestroyCommandPool(m_device, m_commandPool, m_allocator);
    vkDestroyCommandPool(m_device, m_transientCommandPool, m_allocator);

    vmaDestroyAllocator(m_vmaAllocator);

    vkDestroyDevice(m_device, m_allocator);

    if (k_enableValidationLayers) {
        destroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, m_allocator);
    }

    vkDestroySurfaceKHR(m_instance, m_surface, m_allocator);
    vkDestroyInstance(m_instance, nullptr);
}

void Renderer::waitForOperations()
{
    vkDeviceWaitIdle(m_device);
}

[[nodiscard]] uint32_t Renderer::drawCallCount() const
{
    return m_drawCallCount;
}

[[nodiscard]] const std::list<Renderer::InstanceDebugInfo>& Renderer::instancesString() const
{
    return m_instancesString;
}

[[nodiscard]] std::vector<Instance>& Renderer::getInstancesForMesh(uint32_t meshID, uint32_t textureID)
{
    const auto key = std::make_pair(meshID, textureID);
    if (m_meshInstances.find(key) == m_meshInstances.end()) {
        m_meshInstances[key] = std::make_unique<std::vector<Vulkan::Instance>>();
    }
    return *m_meshInstances[key];
}

Texture Renderer::createTextureResources(TextureData& textureData)
{
    Texture texture;
    createTextureImage(texture, textureData);
    createTextureImageView(texture, textureData);
    createTextureSampler(texture, textureData);

    return texture;
}

void Renderer::clearMeshInstances()
{
    m_meshInstances.clear();
}

void Renderer::copyInstancesIntoInstanceBuffer()
{
    static auto& assetManager = g_engine->assetManager();

    for (auto& [meshTextureID, instanceData] : m_meshInstances) {
        if (instanceData->empty()) {
            continue;
        }
        const auto [meshID, textureID] = meshTextureID;
        auto& mesh = assetManager.getMesh(meshID);
        auto& instanceBuffer = mesh.instanceBuffers[textureID];

        instanceBuffer.instanceCount = static_cast<uint32_t>(instanceData->size());
        const uint32_t instanceSize = instanceBuffer.instanceCount * sizeof(Instance);

        // If instance count exceeds size, recreate instance buffer with
        // sufficient size
        if (instanceSize > instanceBuffer.size) {
            vmaDestroyBuffer(m_vmaAllocator, instanceBuffer.vmaBuffer.buffer, instanceBuffer.vmaBuffer.allocation);
            vmaDestroyBuffer(m_vmaAllocator, instanceBuffer.stagingBuffer.buffer, instanceBuffer.stagingBuffer.allocation);

            // Allocate staging buffer in host visible memory
            createBuffer(instanceSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 0, VMA_MEMORY_USAGE_AUTO,
                         VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
                         instanceBuffer.stagingBuffer);

            // Allocate instance buffer in local device memory
            createBuffer(instanceSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 0, VMA_MEMORY_USAGE_AUTO,
                         VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT, instanceBuffer.vmaBuffer);

            instanceBuffer.size = instanceSize;
        }

        // Fill in host-visible buffer
        memcpy(instanceBuffer.stagingBuffer.allocationInfo.pMappedData, instanceData->data(),
               instanceBuffer.instanceCount * sizeof(Instance));

        // Copy data from host-visible staging buffer into local device instance
        // buffer using command queue
        copyBuffer(instanceBuffer.stagingBuffer, instanceBuffer.vmaBuffer, instanceSize);

        // Set descriptors for instance buffer
        instanceBuffer.descriptor.range = instanceBuffer.size;
        instanceBuffer.descriptor.buffer = instanceBuffer.vmaBuffer.buffer;
        instanceBuffer.descriptor.offset = 0;
    }
}

VKAPI_ATTR VkBool32 VKAPI_CALL Renderer::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                       VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                       const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
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
        // RDE_LOG_CLEAN_INFO("Validation Layer:\n\t{}",
        // pCallbackData->pMessage);
        break;
    }
    }
    return VK_FALSE;
}

// Our own function wrapped around the API extension function since it needs to
// be loaded from its address
VkResult Renderer::createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                                const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void Renderer::destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
                                             const VkAllocationCallbacks* pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

[[nodiscard]] std::vector<VkExtensionProperties> Renderer::retrieveSupportedExtensionsList() const
{
    uint32_t extensionCount = 0;

    // Retrieve number of supported extensions
    auto result = vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    RDE_ASSERT_0(result == VK_SUCCESS, "Failed to retrieve instance extension count!");

    std::vector<VkExtensionProperties> extensions(extensionCount);

    // Query extension details
    result = vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
    RDE_ASSERT_0(result == VK_SUCCESS, "Failed to retrieve instance extension list!");

    // std::ostringstream ss;
    // ss << "\nAvailable extensions: \n";
    //
    // for (const auto& extension : extensions) {
    //	ss << '\t' << extension.extensionName << '\n';
    // }
    // RDE_LOG_INFO(ss.str());

    return extensions;
}

[[nodiscard]] std::vector<const char*> Renderer::retrieveRequiredExtensions() const
{
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (k_enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
    return extensions;
}

[[nodiscard]] VkFormat Renderer::retrieveDepthFormat() const
{
    return selectSupportedFormat({VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT}, VK_IMAGE_TILING_OPTIMAL,
                                 VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

VkSampleCountFlagBits Renderer::retrieveMaxSampleCount() const
{
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(m_physicalDevice, &properties);

    VkSampleCountFlags counts = properties.limits.framebufferColorSampleCounts & properties.limits.framebufferDepthSampleCounts;

    return counts & VK_SAMPLE_COUNT_64_BIT   ? VK_SAMPLE_COUNT_64_BIT
           : counts & VK_SAMPLE_COUNT_32_BIT ? VK_SAMPLE_COUNT_32_BIT
           : counts & VK_SAMPLE_COUNT_16_BIT ? VK_SAMPLE_COUNT_16_BIT
           : counts & VK_SAMPLE_COUNT_8_BIT  ? VK_SAMPLE_COUNT_8_BIT
           : counts & VK_SAMPLE_COUNT_4_BIT  ? VK_SAMPLE_COUNT_4_BIT
           : counts & VK_SAMPLE_COUNT_2_BIT  ? VK_SAMPLE_COUNT_2_BIT
                                             : VK_SAMPLE_COUNT_1_BIT;
}

[[nodiscard]] bool Renderer::checkGlfwExtensions(const std::vector<VkExtensionProperties>& supportedExtensions,
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

[[nodiscard]] bool Renderer::checkValidationLayerSupport() const
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

    // If required extensions are empty, it means that available extensions
    // checked them off
    return requiredExtensions.empty();
}

[[nodiscard]] bool Renderer::isDeviceSuitable(VkPhysicalDevice device) const
{
    VkPhysicalDeviceProperties deviceProperties{};
    VkPhysicalDeviceFeatures deviceFeatures{};
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

[[nodiscard]] bool Renderer::hasStencilComponent(VkFormat format)
{
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

[[nodiscard]] bool Renderer::isMsaaEnabled() const
{
    return (m_msaaSamples & VK_SAMPLE_COUNT_1_BIT) != VK_SAMPLE_COUNT_1_BIT;
}

[[nodiscard]] QueueFamilyIndices Renderer::queryQueueFamilies(VkPhysicalDevice device) const
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
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
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

    VkExtent2D actualExtent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

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

[[nodiscard]] VkFormat Renderer::selectSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling,
                                                       VkFormatFeatureFlags features) const
{
    auto hasFeature = [&](VkFormatFeatureFlags propFeature) -> bool { return (propFeature & features) == features; };

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
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
    createInfo.pUserData = nullptr;
}

void Renderer::configureInstanceCreateInfo(VkInstanceCreateInfo& createInfo, const VkApplicationInfo& appInfo,
                                           VkDebugUtilsMessengerCreateInfoEXT& debugMessengerCreateInfo,
                                           const std::vector<const char*>& glfwExtensions) const
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
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugMessengerCreateInfo;
    } else {
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
    appInfo.apiVersion = m_apiVersion;

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

    if (!k_enableValidationLayers)
        return;

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
            m_maxMsaaSamples = retrieveMaxSampleCount();
            RDE_LOG_INFO("Max MSAA Samples available: {}", m_maxMsaaSamples);

            VkPhysicalDeviceProperties properties{};
            vkGetPhysicalDeviceProperties(m_physicalDevice, &properties);
            RDE_LOG_INFO("Physical Device: {}", properties.deviceName);

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
    std::unordered_set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

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
    } else {
        createInfo.enabledLayerCount = 0;
    }

    auto result = vkCreateDevice(m_physicalDevice, &createInfo, m_allocator, &m_device);
    RDE_ASSERT_0(result == VK_SUCCESS, "Failed to create logical device!");

    // Cache device queue
    vkGetDeviceQueue(m_device, indices.graphicsFamily.value(), 0, &m_graphicsQueue);
    vkGetDeviceQueue(m_device, indices.presentFamily.value(), 0, &m_presentQueue);
}

void Renderer::createVmaAllocator()
{
    VmaAllocatorCreateInfo allocatorInfo{};
    allocatorInfo.physicalDevice = m_physicalDevice;
    allocatorInfo.device = m_device;
    allocatorInfo.instance = m_instance;
    allocatorInfo.vulkanApiVersion = m_apiVersion;

    vmaCreateAllocator(&allocatorInfo, &m_vmaAllocator);
}

void Renderer::createSwapchain()
{
    RDE_PROFILE_SCOPE

    Swapchain::SupportDetails swapchainSupport = querySwapchainSupport(m_physicalDevice);

    VkSurfaceFormatKHR surfaceFormat = selectSwapSurfaceFormat(swapchainSupport.formats);
    VkPresentModeKHR presentMode = selectSwapPresentMode(swapchainSupport.presentModes);
    VkExtent2D extent = selectSwapExtent(swapchainSupport.capabilities);
    uint32_t imageCount = swapchainSupport.capabilities.minImageCount + 1;

    // If maxImageCount is 0, that means there's no maximum number of images
    // allowed If there's a max, clamp imageCount to the max
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
    createInfo.imageArrayLayers = 1;                             // Amount of layers each image consists of
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // Directly render to swapchain

    QueueFamilyIndices indices = queryQueueFamilies(m_physicalDevice);

    if (indices.graphicsFamily != indices.presentFamily) {
        uint32_t indicesArray[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT; // Multiple queue families own image
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = indicesArray;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; // One queue family own image exclusively
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }

    createInfo.preTransform = swapchainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE; // Clip pixels that are obscured (by another
                                  // window for example)
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

    RDE_LOG_INFO("MSAA enabled? {}, Samples: {}", isMsaaEnabled() ? "Yes" : "No", m_msaaSamples);

    VkAttachmentDescription colorAttachment{};
    VkAttachmentReference colorAttachmentRef{};

    // Color attachment
    colorAttachment.format = m_swapchain.imageFormat;
    colorAttachment.samples = m_msaaSamples;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // Clear framebuffer to black
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = isMsaaEnabled() ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    colorAttachmentRef.attachment = 0; // Index
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // Depth stencil attachment
    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = retrieveDepthFormat();
    depthAttachment.samples = m_msaaSamples;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;        // We won't use this attachment after
                                                                       // drawing is finished
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;   // For stencil tests
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; // For stencil tests
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1; // Index
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription colorAttachmentResolve{};
    VkAttachmentReference colorAttachmentResolveRef{};

    // Color resolve attachment (from MSAA)
    colorAttachmentResolve.format = m_swapchain.imageFormat;
    colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    colorAttachmentResolveRef.attachment = 2; // Index
    colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // Subpass
    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    if (isMsaaEnabled()) {
        subpass.pResolveAttachments = &colorAttachmentResolveRef;
    }

    // Dependency
    VkSubpassDependency dependency{};
    // This refers to implicit subpass BEFORE render pass
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    // Subpass index, which is 0 since we only have 1 for now
    dependency.dstSubpass = 0;
    // Wait for color attachment output and early fragment tests
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;

    // Wait for this stage to finish to allow writing operations
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    std::vector<VkAttachmentDescription> attachments = {colorAttachment, depthAttachment};

    if (isMsaaEnabled()) {
        attachments.emplace_back(colorAttachmentResolve);
    }

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
    RDE_PROFILE_SCOPE

    // Idea: Could have 4 descriptor sets instead:
    // 0. Engine-global resources - bound once per frame
    // 1. Per-pass resources - bound once per pass
    // 2. Material resources - bound once per material instance
    // 3. Per-object resources - bound once per object instance

    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.binding = 0;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo uboLayoutInfo{};
    uboLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    uboLayoutInfo.bindingCount = 1;
    uboLayoutInfo.pBindings = &uboLayoutBinding;

    VkDescriptorSetLayoutCreateInfo samplerLayoutInfo{};
    samplerLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    samplerLayoutInfo.bindingCount = 1;
    samplerLayoutInfo.pBindings = &samplerLayoutBinding;

    auto result = vkCreateDescriptorSetLayout(m_device, &uboLayoutInfo, m_allocator, &m_uboDescriptorSetLayout);
    RDE_ASSERT_0(result == VK_SUCCESS, "Failed to create UBO descriptor set layout!");
    result = vkCreateDescriptorSetLayout(m_device, &samplerLayoutInfo, m_allocator, &m_samplerDescriptorSetLayout);
    RDE_ASSERT_0(result == VK_SUCCESS, "Failed to create sampler descriptor set layout!");
}

void Renderer::createFramebuffers()
{
    RDE_PROFILE_SCOPE

    m_swapchain.framebuffers.resize(m_swapchain.imageViews.size());

    // Create framebuffer for each image view
    for (size_t i = 0; i < m_swapchain.imageViews.size(); ++i) {
        std::vector<VkImageView> attachments;

        if (isMsaaEnabled()) {
            attachments = std::vector<VkImageView>{m_colorImageView, m_depthImageView, m_swapchain.imageViews[i]};
        } else {
            attachments = std::vector<VkImageView>{m_swapchain.imageViews[i], m_depthImageView};
        }

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

    createImage(m_swapchain.extent.width, m_swapchain.extent.height, 1, m_msaaSamples, colorFormat, VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                m_colorImage, m_colorImageMemory);

    m_colorImageView = createImageView(m_colorImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
}

void Renderer::createDepthResources()
{
    VkFormat depthFormat = retrieveDepthFormat();
    createImage(m_swapchain.extent.width, m_swapchain.extent.height, 1, m_msaaSamples, depthFormat, VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_depthImage, m_depthImageMemory);

    m_depthImageView = createImageView(m_depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);

    transitionImageLayout(m_depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1);
}

void Renderer::loadTextures()
{
    static auto& assetManager = g_engine->assetManager();
    assetManager.loadTextures(k_textureDirPath.c_str());
}

void Renderer::loadModels()
{
    auto& assetManager = g_engine->assetManager();
    assetManager.loadModels(k_modelDirPath.c_str());
}

void Renderer::createVertexBuffers()
{
    auto& assetManager = g_engine->assetManager();
    assetManager.eachMesh([this](Mesh& mesh) { createVertexBuffer(mesh.vertices, mesh.vertexBuffer); });
}

void Renderer::createIndexBuffers()
{
    auto& assetManager = g_engine->assetManager();
    assetManager.eachMesh([this](Mesh& mesh) { createIndexBuffer(mesh.indices, mesh.indexBuffer); });

    assetManager;
}

void Renderer::createVertexBuffer(const std::vector<Vertex>& vertices, VmaBuffer& vertexBuffer)
{
    RDE_PROFILE_SCOPE

    // Temporary host-visible staging buffer
    VmaBuffer stagingBuffer;

    VkDeviceSize bufferSize = Utilities::arraysizeof(vertices);

    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 0, VMA_MEMORY_USAGE_AUTO,
                 VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT, stagingBuffer);

    // Copy data into host-visible staging buffer
    memcpy(stagingBuffer.allocationInfo.pMappedData, vertices.data(), (size_t)bufferSize);

    // Allocate vertex buffer in local device memory
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 0, VMA_MEMORY_USAGE_AUTO,
                 VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT, vertexBuffer);

    // Copy data from host-visible staging buffer into local device vertex
    // buffer using command queue
    copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

    // Clean up temporary staging buffer
    vmaDestroyBuffer(m_vmaAllocator, stagingBuffer.buffer, stagingBuffer.allocation);
}

void Renderer::createIndexBuffer(const std::vector<uint32_t>& indices, VmaBuffer& indexBuffer)
{
    RDE_PROFILE_SCOPE

    // Temporary host-visible buffer
    VmaBuffer stagingBuffer{};
    VkDeviceSize bufferSize = Utilities::arraysizeof(indices);

    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 0, VMA_MEMORY_USAGE_AUTO,
                 VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT, stagingBuffer);

    // Copy data into host-visible staging buffer
    memcpy(stagingBuffer.allocationInfo.pMappedData, indices.data(), (size_t)bufferSize);

    // Allocate vertex buffer in local device memory
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, 0, VMA_MEMORY_USAGE_AUTO, 0, indexBuffer);

    // Copy data from host-visible staging buffer into local device vertex
    // buffer using command queue
    copyBuffer(stagingBuffer, indexBuffer, bufferSize);

    // Clean up temporary staging buffer
    vmaDestroyBuffer(m_vmaAllocator, stagingBuffer.buffer, stagingBuffer.allocation);
}

void Renderer::createInstanceBuffer(InstanceBuffer& instanceBuffer)
{
    // Default to fit 1024 instances first
    constexpr auto initialInstanceCount = 1024;
    instanceBuffer.size = initialInstanceCount * sizeof(Instance);

    // Allocate staging buffer in host visible memory
    createBuffer(instanceBuffer.size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 0, VMA_MEMORY_USAGE_AUTO,
                 VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT, instanceBuffer.stagingBuffer);

    // Allocate instance buffer in VRAM
    createBuffer(instanceBuffer.size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 0, VMA_MEMORY_USAGE_AUTO,
                 VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT, instanceBuffer.vmaBuffer);
}

void Renderer::createUniformBuffers()
{
    RDE_PROFILE_SCOPE

    VkDeviceSize bufferSize = sizeof(UniformBufferObject);
    const auto swapchainDuplicates = m_swapchain.images.size();

    m_uniformBuffers.resize(swapchainDuplicates);

    for (size_t i = 0; i < swapchainDuplicates; ++i) {
        createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 0, VMA_MEMORY_USAGE_AUTO,
                     VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT, m_uniformBuffers[i]);
    }
}

void Renderer::createDescriptorPool()
{
    RDE_PROFILE_SCOPE

    const auto swapchainCount = static_cast<uint32_t>(m_swapchain.images.size());
    constexpr uint32_t maxDescriptorCount = 128;

    VkDescriptorPoolSize uboPoolSize{};
    uboPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboPoolSize.descriptorCount = maxDescriptorCount * swapchainCount;

    VkDescriptorPoolSize samplerPoolSize{};
    samplerPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerPoolSize.descriptorCount = maxDescriptorCount * swapchainCount;

    std::array<VkDescriptorPoolSize, 2> poolSizes = {std::move(uboPoolSize), std::move(samplerPoolSize)};

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = maxDescriptorCount * swapchainCount;
    poolInfo.flags = 0;

    auto result = vkCreateDescriptorPool(m_device, &poolInfo, m_allocator, &m_descriptorPool);
    RDE_ASSERT_0(result == VK_SUCCESS, "Failed to create descriptor pool!");
}

void Renderer::createDescriptorSets()
{
    RDE_PROFILE_SCOPE

    const auto swapchainCount = static_cast<uint32_t>(m_swapchain.images.size());

    // Create vector of swapchain images number of same layouts
    std::vector<VkDescriptorSetLayout> uboLayouts(swapchainCount, m_uboDescriptorSetLayout);

    VkDescriptorSetAllocateInfo uboAllocateInfo{};
    uboAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    uboAllocateInfo.descriptorPool = m_descriptorPool;
    uboAllocateInfo.descriptorSetCount = swapchainCount;
    uboAllocateInfo.pSetLayouts = uboLayouts.data();

    m_uboDescriptorSets.resize(swapchainCount);

    const auto result = vkAllocateDescriptorSets(m_device, &uboAllocateInfo, m_uboDescriptorSets.data());
    RDE_ASSERT_2(result == VK_SUCCESS, "Failed to allocate UBO descriptor sets!");

    for (size_t i = 0; i < swapchainCount; ++i) {
        // For uniform buffer
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = m_uniformBuffers[i].buffer;
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        VkWriteDescriptorSet uboDescriptorWrite{};
        uboDescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        uboDescriptorWrite.dstSet = m_uboDescriptorSets[i];
        uboDescriptorWrite.dstBinding = 0;
        uboDescriptorWrite.dstArrayElement = 0;
        uboDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboDescriptorWrite.descriptorCount = 1;
        uboDescriptorWrite.pBufferInfo = &bufferInfo;
        uboDescriptorWrite.pImageInfo = nullptr;
        uboDescriptorWrite.pTexelBufferView = nullptr;

        vkUpdateDescriptorSets(m_device, 1, &uboDescriptorWrite, 0, nullptr);
    }
    // For texture sampler
    static auto& assetManager = g_engine->assetManager();

    assetManager.eachTexture([this, swapchainCount](Texture& texture, uint32_t id) {
        texture.descriptorSets.resize(swapchainCount);

        std::vector<VkDescriptorSetLayout> samplerLayouts(swapchainCount, m_samplerDescriptorSetLayout);
        VkDescriptorSetAllocateInfo samplerAllocateInfo{};
        samplerAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        samplerAllocateInfo.descriptorPool = m_descriptorPool;
        samplerAllocateInfo.descriptorSetCount = swapchainCount;
        samplerAllocateInfo.pSetLayouts = samplerLayouts.data();

        const auto result = vkAllocateDescriptorSets(m_device, &samplerAllocateInfo, texture.descriptorSets.data());
        RDE_ASSERT_2(result == VK_SUCCESS, "Failed to allocate sampler descriptor sets for texture id {}!", id);

        for (size_t i = 0; i < swapchainCount; ++i) {
            VkDescriptorImageInfo descriptor{};
            descriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            descriptor.imageView = texture.imageView;
            descriptor.sampler = texture.sampler;

            VkWriteDescriptorSet samplerDescriptorWrite{};
            samplerDescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            samplerDescriptorWrite.dstSet = texture.descriptorSets[i];
            samplerDescriptorWrite.dstBinding = 0;
            samplerDescriptorWrite.dstArrayElement = 0;
            samplerDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            samplerDescriptorWrite.descriptorCount = 1;
            samplerDescriptorWrite.pBufferInfo = nullptr;
            samplerDescriptorWrite.pImageInfo = &descriptor;
            samplerDescriptorWrite.pTexelBufferView = nullptr;

            vkUpdateDescriptorSets(m_device, 1, &samplerDescriptorWrite, 0, nullptr);
        }
    });
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

        auto success = vkCreateSemaphore(m_device, &semaphoreInfo, m_allocator, &m_imageAvailableSemaphores[i]) == VK_SUCCESS &&
                       vkCreateSemaphore(m_device, &semaphoreInfo, m_allocator, &m_renderFinishedSemaphores[i]) == VK_SUCCESS &&
                       vkCreateFence(m_device, &fenceInfo, m_allocator, &m_inFlightFences[i]) == VK_SUCCESS;

        RDE_ASSERT_0(success, "Failed to create synchronization objects for frame {}!", i);
    }
}

void Renderer::initImGui()
{
    // Create descriptor pool for ImGui to use (Type, DescriptorCount)
    constexpr uint32_t descriptorCount = 1000;

    VkDescriptorPoolSize poolSizes[] = {{VK_DESCRIPTOR_TYPE_SAMPLER, descriptorCount},
                                        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, descriptorCount},
                                        {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, descriptorCount},
                                        {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, descriptorCount},
                                        {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, descriptorCount},
                                        {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, descriptorCount},
                                        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, descriptorCount},
                                        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, descriptorCount},
                                        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, descriptorCount},
                                        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, descriptorCount},
                                        {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, descriptorCount}};

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolInfo.maxSets = descriptorCount;
    poolInfo.poolSizeCount = static_cast<uint32_t>(std::size(poolSizes));
    poolInfo.pPoolSizes = poolSizes;

    RDE_ASSERT_0(vkCreateDescriptorPool(m_device, &poolInfo, m_allocator, &m_imguiDescriptorPool) == VK_SUCCESS,
                 "Failed to create descriptor pool for ImGui!");

    // Initialize ImGui library
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForVulkan(g_engine->window().apiWindow(), true);

    ImGui_ImplVulkan_InitInfo initInfo{};
    initInfo.Instance = m_instance;
    initInfo.PhysicalDevice = m_physicalDevice;
    initInfo.Device = m_device;
    initInfo.Queue = m_graphicsQueue;
    initInfo.DescriptorPool = m_imguiDescriptorPool;
    initInfo.MinImageCount = k_maxFramesInFlight;
    initInfo.ImageCount = k_maxFramesInFlight;
    initInfo.MSAASamples = m_msaaSamples;

    ImGui_ImplVulkan_Init(&initInfo, m_renderPass);

    singleTimeCommands([&](VkCommandBuffer commandBuffer) { ImGui_ImplVulkan_CreateFontsTexture(commandBuffer); });
}

[[nodiscard]] VkImageView Renderer::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags,
                                                    uint32_t mipLevels) const
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

void Renderer::createBuffer(VkDeviceSize size, VkBufferUsageFlags bufferUsage, VkBufferCreateFlags bufferFlags,
                            VmaMemoryUsage allocationUsage, VmaAllocationCreateFlags allocationFlags, VmaBuffer& vmaBuffer) const
{
    // Create buffer
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = bufferUsage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // Only allow one queue to use
    bufferInfo.flags = bufferFlags;

    VmaAllocationCreateInfo allocationInfo{};
    allocationInfo.usage = allocationUsage;
    allocationInfo.flags = allocationFlags;

    // vmaCreateBuffer encapsulates the following:
    // 1. Create VkBuffer
    // 2. Allocate VkBufferMemory
    // 3. Bind VkBuffer to VkBufferMemory
    const auto result =
        vmaCreateBuffer(m_vmaAllocator, &bufferInfo, &allocationInfo, &vmaBuffer.buffer, &vmaBuffer.allocation, &vmaBuffer.allocationInfo);
    RDE_ASSERT_0(result == VK_SUCCESS, "Failed to create buffer!");
}

void Renderer::createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits sampleCount, VkFormat format,
                           VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image,
                           VkDeviceMemory& imageMemory) const
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

    // vmaCreateImage();
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

    RDE_ASSERT_0(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT,
                 "Texture image format does not support linear blitting!");

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
                vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0,
                                     nullptr, 1, &barrier);

                VkImageBlit blit{};
                blit.srcOffsets[0] = {0, 0, 0};
                blit.srcOffsets[1] = {mipWidth, mipHeight, 1};
                blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                blit.srcSubresource.mipLevel = i - 1;
                blit.srcSubresource.baseArrayLayer = 0;
                blit.srcSubresource.layerCount = 1;
                blit.dstOffsets[0] = {0, 0, 0};
                blit.dstOffsets[1] = {mipWidth > 1 ? mipWidth >> 1 : 1, mipHeight > 1 ? mipHeight >> 1 : 1, 1};
                blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                blit.dstSubresource.mipLevel = i;
                blit.dstSubresource.baseArrayLayer = 0;
                blit.dstSubresource.layerCount = 1;

                // Blit the image
                vkCmdBlitImage(commandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                               &blit, VK_FILTER_LINEAR);

                barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

                // Wait for layout transition
                vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0,
                                     nullptr, 1, &barrier);

                if (mipWidth > 1)
                    mipWidth /= 2;
                if (mipHeight > 1)
                    mipHeight /= 2;
            }
        }
        barrier.subresourceRange.baseMipLevel = mipLevels - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        // Wait for final mip level
        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0,
                             nullptr, 1, &barrier);
    });
}

void Renderer::createTextureImage(Texture& texture, TextureData& textureData)
{
    RDE_PROFILE_SCOPE

    VkDeviceSize imageSize = textureData.texWidth * textureData.texHeight * STBI_rgb_alpha;

    // Set miplevels
    texture.mipLevels =
        m_enableMipmaps ? static_cast<uint32_t>(std::floor(std::log2(std::max(textureData.texWidth, textureData.texHeight)))) + 1 : 1;

    VmaBuffer stagingBuffer;

    createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 0, VMA_MEMORY_USAGE_AUTO,
                 VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT, stagingBuffer);

    memcpy(stagingBuffer.allocationInfo.pMappedData, textureData.data, static_cast<size_t>(imageSize));

    createImage(textureData.texWidth, textureData.texHeight, texture.mipLevels, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB,
                VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, texture.image, texture.imageMemory);

    transitionImageLayout(texture.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                          texture.mipLevels);
    copyBufferToImage(stagingBuffer, texture.image, static_cast<uint32_t>(textureData.texWidth),
                      static_cast<uint32_t>(textureData.texHeight));
    generateMipmaps(texture.image, VK_FORMAT_R8G8B8A8_SRGB, textureData.texWidth, textureData.texHeight, texture.mipLevels);

    vmaDestroyBuffer(m_vmaAllocator, stagingBuffer.buffer, stagingBuffer.allocation);
}

void Renderer::createTextureImageView(Texture& texture, TextureData& textureData)
{
    RDE_PROFILE_SCOPE

    texture.imageView = createImageView(texture.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, texture.mipLevels);
}

void Renderer::createTextureSampler(Texture& texture, TextureData& textureData)
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
    samplerInfo.maxLod = m_enableMipmaps ? static_cast<float>(texture.mipLevels) : VK_LOD_CLAMP_NONE;

    RDE_ASSERT_0(vkCreateSampler(m_device, &samplerInfo, m_allocator, &texture.sampler) == VK_SUCCESS, "Failed to create texture sampler!");
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
    m_pipeline.destroy(m_device, m_allocator);
    vkDestroyRenderPass(m_device, m_renderPass, m_allocator);

    for (auto imageView : m_swapchain.imageViews) {
        vkDestroyImageView(m_device, imageView, m_allocator);
    }

    vkDestroySwapchainKHR(m_device, m_swapchain.handle, m_allocator);

    for (size_t i = 0; i < m_swapchain.images.size(); ++i) {
        vmaDestroyBuffer(m_vmaAllocator, m_uniformBuffers[i].buffer, m_uniformBuffers[i].allocation);
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
    m_pipeline.create(m_device, m_allocator, m_swapchain, m_msaaSamples, m_uboDescriptorSetLayout, m_samplerDescriptorSetLayout,
                      m_renderPass);
    createColorResources();
    createDepthResources();
    createFramebuffers();
    createUniformBuffers();
    createDescriptorPool();
    createDescriptorSets();
    createCommandBuffers();
}

void Renderer::cleanUpImGui()
{
    ImGui_ImplVulkan_DestroyFontUploadObjects();
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    vkDestroyDescriptorPool(m_device, m_imguiDescriptorPool, m_allocator);
}

void Renderer::copyBuffer(const VmaBuffer& srcBuffer, VmaBuffer& dstBuffer, VkDeviceSize size)
{
    singleTimeCommands([&](VkCommandBuffer commandBuffer) {
        VkBufferCopy copyRegion{};
        copyRegion.srcOffset = 0;
        copyRegion.dstOffset = 0;
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, srcBuffer.buffer, dstBuffer.buffer, 1, &copyRegion);
    });
}

void Renderer::updateUniformBuffer(uint32_t imageIndex)
{
    UniformBufferObject ubo{};

    const auto& camera = g_engine->scene().camera();

    ubo.view = glm::lookAt(camera.eye, camera.eye + camera.front, camera.up);
    ubo.projection = glm::perspective(glm::radians(camera.fov), m_swapchain.extent.width / (float)m_swapchain.extent.height,
                                      camera.nearClip, camera.farClip);

    // Flip Y
    ubo.projection[1][1] *= -1.0f;

    memcpy(m_uniformBuffers[imageIndex].allocationInfo.pMappedData, &ubo, sizeof(ubo));
}

void Renderer::recordCommandBuffers(uint32_t imageIndex)
{
    VkResult result;

    // Explicitly reset command buffer to initial state
    RDE_ASSERT_2(vkResetCommandBuffer(m_commandBuffers[imageIndex], VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT) == VK_SUCCESS,
                 "Failed to reset command buffer!");

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
        renderPassBeginInfo.renderArea.offset = {0, 0};
        renderPassBeginInfo.renderArea.extent = m_swapchain.extent;

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {k_clearColor.x, k_clearColor.y, k_clearColor.z, k_clearColor.w};
        clearValues[1].depthStencil = {1.0f, 0};

        renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassBeginInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(m_commandBuffers[imageIndex], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        // Bind graphics pipeline
        m_pipeline.bind(m_commandBuffers[imageIndex]);

        // Bind UBO descriptor set
        vkCmdBindDescriptorSets(m_commandBuffers[imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline.layout(), /* firstSet */ 0,
                                /* descriptorSetCount */ 1, m_uboDescriptorSets.data(), /* dynamicOffsetCount */ 0,
                                /* pDynamicOffsets */ nullptr);

        static auto& assetManager = g_engine->assetManager();
        m_instancesString.clear();

        // For each mesh and texture, bind texture sampler descriptor set and draw instanced
        for (const auto& [meshTextureId, instance] : m_meshInstances) {
            const auto [meshId, textureId] = meshTextureId;
            const auto& mesh = assetManager.getMesh(meshId);
            const auto& texture = assetManager.getTexture(textureId);

            vkCmdBindDescriptorSets(m_commandBuffers[imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline.layout(), /* firstSet */ 1,
                                    /* descriptorSetCount */ 1, &texture.descriptorSets[imageIndex], /* dynamicOffsetCount */ 0,
                                    /* pDynamicOffsets */ nullptr);

            const auto& instanceBuffer = mesh.instanceBuffers.at(textureId);
            drawCommand(m_commandBuffers[imageIndex], mesh.vertexBuffer.buffer, mesh.indexBuffer.buffer, instanceBuffer,
                        static_cast<uint32_t>(mesh.indices.size()));

            // For debugging and to show on ImGui
            const auto& meshName = assetManager.getAssetName(meshId);
            const auto& textureName = assetManager.getAssetName(textureId);
            m_instancesString.emplace_back(std::make_tuple(meshName, textureName, instanceBuffer.instanceCount));
        }

        // Render ImGui draw data (Need to check in case ImGui is not running)
        if (g_engine->editor().renderingEnabled()) {
            auto* drawData = ImGui::GetDrawData();
            if (drawData) {
                ImGui_ImplVulkan_RenderDrawData(drawData, m_commandBuffers[imageIndex]);
            }
        }

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
        } else {
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
        } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        } else {
            RDE_ASSERT_0(false, "Unsupported layout transition used!");
        }

        vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
    });
}

void Renderer::copyBufferToImage(const VmaBuffer& buffer, VkImage image, uint32_t width, uint32_t height)
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

        region.imageOffset = {0, 0, 0};
        region.imageExtent = {width, height, 1};

        vkCmdCopyBufferToImage(commandBuffer, buffer.buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    });
}

VkCommandBuffer Renderer::beginSingleTimeCommands()
{
    VkCommandBufferAllocateInfo allocateInfo{};
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocateInfo.commandPool = m_transientCommandPool; // Use transientCommandPool for short-lived
                                                       // command buffers
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

void Renderer::drawCommand(VkCommandBuffer commandBuffer, const VkBuffer& vertexBuffer, const VkBuffer& indexBuffer,
                           const InstanceBuffer& instanceBuffer, uint32_t indexCount)
{
    if (!instanceBuffer.instanceCount) {
        return;
    }

    // TODO: Batch all VBs and IBs into one and use indexing
    VkDeviceSize offsets[] = {0};

    vkCmdBindVertexBuffers(commandBuffer, VertexBufferBindingID, 1, &vertexBuffer, offsets);
    vkCmdBindVertexBuffers(commandBuffer, InstanceBufferBindingID, 1, &instanceBuffer.vmaBuffer.buffer, offsets);

    static_assert(std::is_same_v<Mesh::IndicesValueType, uint16_t> || std::is_same_v<Mesh::IndicesValueType, uint32_t>,
                  "Index buffer is not uint32_t or uint16_t!");

    // Bind index buffer for this mesh
    if constexpr (std::is_same_v<Mesh::IndicesValueType, std::uint16_t>) {
        vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT16);
    } else {
        vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
    }

    // vkCmdPushConstants(commandBuffer, m_pipelineLayout,
    // VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4),
    // &m_pushConstants.modelMtx);

    // Draw command for this mesh
    vkCmdDrawIndexed(commandBuffer, indexCount, instanceBuffer.instanceCount, 0, 0, 0);
    ++m_drawCallCount;
}
} // namespace Vulkan
} // namespace RDE
