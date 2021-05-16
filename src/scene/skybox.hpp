#pragma once

#include "render/buffer.hpp"

#include <glm/glm.hpp>
#include <string_view>
#include <vulkan/vulkan.h>

namespace shady::scene {

struct SkyboxUBO
{
   glm::mat4 view_projection;
};

class Skybox
{
 public:
   void
   LoadCubeMap(std::string_view directory);

   void
   Draw(VkCommandBuffer commandBuffer);

   void
   UpdateBuffers();

 private:
   void
   CreatePipeline();

   void
   CreateDescriptorSet();

   void
   CreateBuffers();

   void
   CreateImageAndSampler(std::string_view directory);

 private:
   VkPipeline m_pipeline = {};
   VkPipelineLayout m_pipelineLayout = {};
   VkDescriptorSetLayout m_descriptorSetLayout = {};
   VkDescriptorSet m_descriptorSet = {};
   VkDescriptorPool m_descriptorPool = {};
   VkImage m_image = {};
   VkDeviceMemory m_imageMemory = {};
   VkImageView m_imageView = {};
   VkSampler m_sampler = {};
   render::Buffer m_vertexBuffer = {};
   render::Buffer m_indexBuffer = {};
   render::Buffer m_uniformBuffer = {};
};

} // namespace shady::scene
