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
   Initialize(VkRenderPass mainRenderPass, const std::vector< VkImageView >& swapChainImageViews,
              VkPipelineCache pipelineCache);

   static VkDescriptorSet&
   GetDescriptorSet();

   static VkPipelineLayout
   GetPipelineLayout();

   static VkPipeline
   GetCompositionPipeline();

   static VkCommandBuffer&
   GetOffscreenCmdBuffer();

   static VkSemaphore&
   GetOffscreenSemaphore();

   static void
   UpdateDeferred(const scene::Camera* camera, const scene::Light* light);

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
   BuildDeferredCommandBuffer(const std::vector< VkImageView >& swapChainImageViews);

   static void
   UpdateUniformBufferComposition(const scene::Camera* camera, const scene::Light* light);

   static void
   UpdateUniformBufferOffscreen(const scene::Camera* camera);

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
   inline static scene::Skybox m_skybox = {};
};

} // namespace shady::render
