#pragma once

#include "vulkan_buffer.hpp"
#include "vulkan_framebuffer.hpp"
#include "scene/camera.hpp"

#include <vector>
#include <vulkan/vulkan.h>
#include <memory>

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

   void
   UpdateUniformBufferComposition();
   void
   UpdateUniformBufferOffscreen();

   inline static Framebuffer m_frameBuffer = {};
   inline static VkPipeline m_graphicsPipeline = {};
   inline static VkPipelineLayout m_pipelineLayout = {};
   inline static std::vector< VkDescriptorSet > m_descriptorSets = {};
   inline static VkDescriptorSet m_descriptorSet = {};
   inline static VkDescriptorSetLayout m_descriptorSetLayout = {};

   inline static Buffer m_offscreenBuffer = {};
   inline static Buffer m_compositionBuffer = {};
   inline static int32_t m_debugDisplayTarget = 0;
   inline static std::unique_ptr<scene::Camera> m_camera = {};
};

} // namespace shady::render::vulkan
