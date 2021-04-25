#pragma once

#include <memory>
#include <unordered_map>
#include <vector>

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

struct GLFWwindow;

namespace shady::app::gui {

class Gui
{
 public:
   void
   Init(GLFWwindow* windowHandle);

   void
   Shutdown();

   void
   Render(const glm::ivec2& windowSize, uint32_t shadowMapID);

 private:
   void
   PrepareResources();
   void
   PreparePipeline(const VkPipelineCache pipelineCache, const VkRenderPass renderPass);

 private:
   VkImage m_fontImage = {};
   VkDeviceMemory m_fontMemory = {};
   VkImageView m_fontView = {};
   VkSampler m_sampler = {};
   VkDescriptorPool m_descriptorPool = {};
   VkDescriptorSetLayout m_descriptorSetLayout = {};
   VkDescriptorSet m_descriptorSet = {};
};

} // namespace shady::app::gui
