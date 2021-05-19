#pragma once

#include "buffer.hpp"

#include <memory>
#include <unordered_map>
#include <vector>

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

namespace shady::scene {
class Scene;
}

namespace shady::app::gui {

struct PushConstBlock
{
   glm::vec2 scale;
   glm::vec2 translate;
};

class Gui
{
 public:
   static void
   Init(const glm::ivec2& windowSize);

   static void
   Shutdown();

   static bool
   UpdateUI(const glm::ivec2& windowSize, scene::Scene& scene);

   static bool
   CheckUpdateUI();

   static bool
   UpdateBuffers();

   static void
   Render(VkCommandBuffer commandBuffer);

 private:
   static void
   PrepareResources();

   static void
   PreparePipeline(const VkPipelineCache pipelineCache, const VkRenderPass renderPass);

 private:
   inline static VkImage m_fontImage = {};
   inline static VkDeviceMemory m_fontMemory = {};
   inline static VkImageView m_fontView = {};
   inline static VkSampler m_sampler = {};
   inline static VkDescriptorPool m_descriptorPool = {};
   inline static VkDescriptorSetLayout m_descriptorSetLayout = {};
   inline static VkDescriptorSet m_descriptorSet = {};

   inline static VkPipeline m_pipeline = {};
   inline static VkPipelineLayout m_pipelineLayout = {};
   inline static uint32_t m_subpass = 0;

   inline static PushConstBlock m_pushConstant = {};
   inline static render::Buffer m_vertexBuffer = {};
   inline static render::Buffer m_indexBuffer = {};
   inline static int32_t m_vertexCount = 0;
   inline static int32_t m_indexCount = 0;
};

} // namespace shady::app::gui
