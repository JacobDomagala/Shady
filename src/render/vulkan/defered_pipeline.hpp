#pragma once

#include "scene/camera.hpp"
#include "vulkan_buffer.hpp"
#include "vulkan_framebuffer.hpp"


#include <memory>
#include <vector>
#include <vulkan/vulkan.h>


namespace shady::render::vulkan {

class DeferedPipeline
{
 public:
   void
   Initialize(VkRenderPass mainRenderPass);

 private:
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
   BuildCommandBuffers();

   void
   BuildDeferredCommandBuffer();

   void
   UpdateUniformBufferComposition();
   void
   UpdateUniformBufferOffscreen();

   inline static VkRenderPass m_mainRenderPass = {};
   inline static VkPipeline m_graphicsPipeline = {};

   inline static Framebuffer m_offscreenFrameBuffer = {};
   inline static Framebuffer m_compositionFrameBuffer = {};

   inline static VkPipelineCache m_pipelineCache = {};
   inline static VkPipeline m_offscreenPipeline = {};
   inline static VkPipeline m_compositionPipeline = {};

   inline static VkPipelineLayout m_pipelineLayout = {};
   inline static std::vector< VkDescriptorSet > m_descriptorSets = {};
   inline static VkDescriptorSet m_descriptorSet = {};
   inline static VkDescriptorSetLayout m_descriptorSetLayout = {};
   inline static VkDescriptorPool m_descriptorPool = {};

   inline static Buffer m_offscreenBuffer = {};
   inline static Buffer m_compositionBuffer = {};
   inline static int32_t m_debugDisplayTarget = 0;
   inline static std::unique_ptr< scene::Camera > m_camera = {};
};

} // namespace shady::render::vulkan
