#pragma once

#include "vulkan_framebuffer.hpp"

#include <vector>
#include <vulkan/vulkan.h>


namespace shady::render::vulkan {

struct
{
   VkPipeline offscreen;
   VkPipeline composition;
} pipelines;

class DeferedPipeline
{
 public:
   void
   Initialize();

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

   inline static Framebuffer m_frameBuffer = {};
   inline static VkPipeline m_graphicsPipeline = {};
   inline static VkPipelineLayout m_pipelineLayout = {};
   inline static std::vector< VkDescriptorSet > m_descriptorSets = {};
   inline static VkDescriptorSet m_descriptorSet = {};
   inline static VkDescriptorSetLayout m_descriptorSetLayout = {};
};

} // namespace shady::render::vulkan
