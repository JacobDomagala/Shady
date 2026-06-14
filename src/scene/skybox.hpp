#pragma once

#include "render/buffer.hpp"

#include <glm/glm.hpp>
#include <string_view>
#include <vector>
#include <vulkan/vulkan.h>

namespace shady::scene {
class Camera;
}

namespace shady::scene {

struct SkyboxUBO
{
   // camera's viewProjection (minus the translation)
   glm::mat4 viewProjection;
};

class Skybox
{
 public:
   /*
    * Load Cubemap from 6 face files. Each face file should be suffixed with face orientation.
    *
    * Example:
    * sky_front.jpg, sky_back.jpg, sky_left.jpg, sky_right.jpg, sky_top.jpg, sky_bottom.jpg
    *
    * param[in]: skyboxName prefix for each face file
    */
   void
   LoadCubeMap(std::string_view skyboxName, VkRenderPass renderPass);

   /*
    *  Draw commands for 'commandBuffer'
    */
   void
   Draw(VkCommandBuffer commandBuffer, uint32_t frame);

   /*
    *  Update uniform buffer (SkyboxUBO)
    */
   void
   UpdateBuffers(const scene::Camera* camera, uint32_t frame);

 private:
   void
   CreatePipeline(VkRenderPass renderPass);

   void
   CreateDescriptorSet();

   void
   CreateBuffers();

   void
   CreateImageAndSampler(std::string_view skyboxName);

 private:
   VkPipeline m_pipeline = {};
   VkPipelineLayout m_pipelineLayout = {};
   VkDescriptorSetLayout m_descriptorSetLayout = {};
   std::vector< VkDescriptorSet > m_descriptorSets = {};
   VkDescriptorPool m_descriptorPool = {};

   VkImage m_image = {};
   VkDeviceMemory m_imageMemory = {};
   VkImageView m_imageView = {};
   VkSampler m_sampler = {};

   render::Buffer m_vertexBuffer = {};
   render::Buffer m_indexBuffer = {};
   std::vector< render::Buffer > m_uniformBuffers = {};
};

} // namespace shady::scene
