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
   UpdateBuffers(uint32_t frameIndex);

   static void
   Render(VkCommandBuffer commandBuffer, uint32_t frameIndex);

 private:
   static void
   PrepareResources();

   static void
   PreparePipeline(VkPipelineCache pipelineCache, VkRenderPass renderPass);

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
   inline static std::vector< render::Buffer > m_vertexBuffers = {};
   inline static std::vector< render::Buffer > m_indexBuffers = {};
   inline static std::vector< int32_t > m_vertexCounts = {};
   inline static std::vector< int32_t > m_indexCounts = {};
};

} // namespace shady::app::gui
