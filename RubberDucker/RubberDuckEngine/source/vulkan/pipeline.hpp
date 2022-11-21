#pragma once
#include "vulkan/swapchain.hpp"

#include <vulkan/vulkan.hpp>

namespace RDE
{
namespace Vulkan
{

class Pipeline
{
  public:
    void create(VkDevice device, VkAllocationCallbacks* allocator, const Swapchain& swapchain, VkSampleCountFlagBits msaaSamples, VkDescriptorSetLayout descriptorSetLayout,
                VkRenderPass renderPass);
    void destroy(VkDevice device, VkAllocationCallbacks* allocator);
    void bind(VkCommandBuffer commandBuffer);

    [[nodiscard]] __forceinline VkPipeline pipeline() const { return m_graphicsPipeline; }
    [[nodiscard]] __forceinline VkPipelineLayout layout() const { return m_pipelineLayout; }

  private:
    [[nodiscard]] VkShaderModule createShaderModule(VkDevice device, VkAllocationCallbacks* allocator, FileParser::FileBufferType shaderCode) const;

    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
    VkPipeline m_graphicsPipeline = VK_NULL_HANDLE;
};

} // namespace Vulkan
} // namespace RDE