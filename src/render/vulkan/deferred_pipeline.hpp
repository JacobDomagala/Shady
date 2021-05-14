#pragma once

#include "vulkan_buffer.hpp"
#include "vulkan_framebuffer.hpp"

#include <unordered_map>
#include <memory>
#include <vector>
#include <vulkan/vulkan.h>


namespace shady::render::vulkan {

class DeferredPipeline
{
 public:
   void
   Initialize(VkRenderPass mainRenderPass, const std::vector< VkImageView >& swapChainImageViews,
              VkPipelineCache pipelineCache);

   VkDescriptorSet&
   GetDescriptorSet();

   VkPipelineLayout
   GetPipelineLayout();

   VkPipeline
   GetCompositionPipeline();

   VkCommandBuffer&
   GetOffscreenCmdBuffer();

   VkSemaphore&
   GetOffscreenSemaphore();

   void
   UpdateDeferred();

 private:
   void
   ShadowSetup();

   // Prepare a new framebuffer and attachments for offscreen rendering (G-Buffer)
   void
   PrepareOffscreenFramebuffer();

   void
   PrepareUniformBuffers();

   void
   SetupDescriptorSetLayout();

   void
   PreparePipelines();

   void
   SetupDescriptorPool();

   void
   SetupDescriptorSet();

   void
   BuildDeferredCommandBuffer(const std::vector< VkImageView >& swapChainImageViews);

   void
   UpdateUniformBufferComposition();
   void
   UpdateUniformBufferOffscreen();

   inline static VkRenderPass m_mainRenderPass = {};
   inline static VkPipeline m_graphicsPipeline = {};

   inline static Framebuffer m_shadowMap = {};
   inline static Framebuffer m_offscreenFrameBuffer = {};
   inline static Framebuffer m_compositionFrameBuffer = {};

   inline static VkPipelineCache m_pipelineCache = {};
   inline static VkPipeline m_shadowMapPipeline = {};
   inline static VkPipeline m_offscreenPipeline = {};
   inline static VkPipeline m_compositionPipeline = {};

   inline static VkPipelineLayout m_pipelineLayout = {};
   inline static std::vector< VkDescriptorSet > m_descriptorSets = {};
   inline static VkDescriptorSet m_descriptorSet = {};
   inline static VkDescriptorSetLayout m_descriptorSetLayout = {};
   inline static VkDescriptorPool m_descriptorPool = {};

   inline static Buffer m_offscreenBuffer = {};
   inline static Buffer m_compositionBuffer = {};
   inline static VkDescriptorSet m_shadowMapDescriptor = {};
   inline static int32_t m_debugDisplayTarget = 0;

   inline static VkSampler m_colorSampler = {};

   inline static std::vector< VkCommandBuffer > m_commandBuffers = {};
   inline static VkCommandBuffer m_offscreenCommandBuffer = {};
   inline static VkSemaphore m_offscreenSemaphore = {};

   inline static VkViewport m_viewport = {};
};

} // namespace shady::render::vulkan
