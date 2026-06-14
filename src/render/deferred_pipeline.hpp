#pragma once

#include "buffer.hpp"
#include "framebuffer.hpp"
#include "scene/skybox.hpp"

#include <memory>
#include <unordered_map>
#include <vector>
#include <vulkan/vulkan.h>

namespace shady::scene {
class Light;
class Camera;
} // namespace shady::scene

namespace shady::render {

class DeferredPipeline
{
 public:
   static void
   Initialize(VkRenderPass mainRenderPass, VkPipelineCache pipelineCache);

   static VkDescriptorSet&
   GetDescriptorSet(uint32_t frame);

   static VkPipelineLayout
   GetPipelineLayout();

   static VkPipeline
   GetCompositionPipeline();

   static void
   DrawSkybox(VkCommandBuffer commandBuffer, uint32_t frame);

   static VkCommandBuffer&
   GetOffscreenCmdBuffer(uint32_t frame);

   static void
   UpdateDeferred(const scene::Camera* camera, const scene::Light* light, uint32_t frame);

   static void
   BuildDeferredCommandBuffer(uint32_t frame);

 private:
   static void
   ShadowSetup();

   // Prepare a new framebuffer and attachments for offscreen rendering (G-Buffer)
   static void
   PrepareOffscreenFramebuffer();

   static void
   PrepareUniformBuffers();

   static void
   SetupDescriptorSetLayout();

   static void
   PreparePipelines();

   static void
   SetupDescriptorPool();

   static void
   SetupDescriptorSet();

   static void
   UpdateUniformBufferComposition(const scene::Camera* camera, const scene::Light* light,
                                   uint32_t frame);

   static void
   UpdateUniformBufferOffscreen(const scene::Camera* camera, uint32_t frame);

   inline static VkRenderPass m_mainRenderPass = {};
   inline static VkPipeline m_graphicsPipeline = {};

   inline static std::vector< Framebuffer > m_shadowMaps = {};
   inline static std::vector< Framebuffer > m_offscreenFrameBuffers = {};

   inline static VkPipelineCache m_pipelineCache = {};
   inline static VkPipeline m_shadowMapPipeline = {};
   inline static VkPipeline m_offscreenPipeline = {};
   inline static VkPipeline m_compositionPipeline = {};

   inline static VkPipelineLayout m_pipelineLayout = {};
   inline static std::vector< VkDescriptorSet > m_descriptorSets = {};
   inline static VkDescriptorSetLayout m_descriptorSetLayout = {};
   inline static VkDescriptorPool m_descriptorPool = {};

   inline static std::vector< Buffer > m_offscreenBuffer = {};
   inline static std::vector< Buffer > m_compositionBuffer = {};
   inline static VkDescriptorSet m_shadowMapDescriptor = {};
   inline static int32_t m_debugDisplayTarget = 0;

   inline static VkSampler m_colorSampler = {};

   inline static std::vector< VkCommandBuffer > m_offscreenCommandBuffer = {};

   inline static VkViewport m_viewport = {};
   inline static scene::Skybox m_skybox = {};
};

} // namespace shady::render
