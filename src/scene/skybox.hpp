#pragma once

#include "render/vulkan/vulkan_buffer.hpp"

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
   render::vulkan::Buffer m_vertexBuffer = {};
   render::vulkan::Buffer m_indexBuffer = {};
   render::vulkan::Buffer m_uniformBuffer = {};
};

} // namespace shady::scene
