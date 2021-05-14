#pragma once

#include "render/vulkan/vulkan_buffer.hpp"
#include "scene/camera.hpp"

#include <string>
#include <vulkan/vulkan.h>

namespace shady::scene {

class Skybox
{
 public:
   void
   LoadCubeMap(const std::string& directoryPath);

   void
   Draw(const Camera& camera, uint32_t windowWidth, uint32_t windowHeight);

 private:
   void
   CreatePipeline();

 private:
   VkPipeline m_pipeline = {};
   VkPipelineLayout m_pipelineLayout = {};
   VkDescriptorSetLayout m_descriptorSetLayout = {};
   render::vulkan::Buffer m_vertexBuffer = {};
};

} // namespace shady::scene
