#pragma once

#include "render/buffer.hpp"

#include <glm/glm.hpp>
#include <string_view>
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
   LoadCubeMap(std::string_view skyboxName);

   /*
    *  Draw commands for 'commandBuffer'
    */
   void
   Draw(VkCommandBuffer commandBuffer);

   /*
    *  Update uniform buffer (SkyboxUBO)
    */
   void
   UpdateBuffers(const scene::Camera* camera);

 private:
   void
   CreatePipeline();

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
