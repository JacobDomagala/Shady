#pragma once

#include <string_view>
#include <utility>
#include <vulkan/vulkan.h>

namespace shady::render::vulkan {

struct ShaderInfoWrapper
{
   /*
    * This should be called after the pipeline is created
    */
   void
   Destroy()
   {
      vkDestroyShaderModule(device, shaderInfo.module, nullptr);
   }

   VkDevice device;
   VkPipelineShaderStageCreateInfo shaderInfo;
};

using VertexShaderInfo = ShaderInfoWrapper;
using GeometryShaderInfo = ShaderInfoWrapper;
using FragmentShaderInfo = ShaderInfoWrapper;

class VulkanShader
{
 public:
   static ShaderInfoWrapper
   LoadShader(std::string_view shader, VkShaderStageFlagBits stage);

   static std::pair< VertexShaderInfo, FragmentShaderInfo >
   CreateShader(VkDevice device, std::string_view vertex, std::string_view fragment);
};

} // namespace shady::render::vulkan
