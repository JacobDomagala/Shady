#include "vulkan_shader.hpp"
#include "trace/logger.hpp"
#include "utils/file_manager.hpp"

namespace shady::render::vulkan {

static VkShaderModule
CreateShaderModule(VkDevice device, std::vector< char >&& shaderBinaryCode)
{
   VkShaderModuleCreateInfo createInfo{};
   createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
   createInfo.codeSize = shaderBinaryCode.size();
   createInfo.pCode = reinterpret_cast< const uint32_t* >(shaderBinaryCode.data());

   VkShaderModule shaderModule;
   if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
   {
      throw std::runtime_error("failed to create shader module!");
   }

   return shaderModule;
}


std::pair< VertexShaderInfo, FragmentShaderInfo >
VulkanShader::CreateShader(VkDevice device, std::string_view vertex, std::string_view fragment)
{
   VkShaderModule vertShaderModule = CreateShaderModule(
      device, utils::FileManager::ReadBinaryFile(utils::FileManager::SHADERS_DIR / vertex));
   VkShaderModule fragShaderModule = CreateShaderModule(
      device, utils::FileManager::ReadBinaryFile(utils::FileManager::SHADERS_DIR / fragment));

   VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
   vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
   vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
   vertShaderStageInfo.module = vertShaderModule;
   vertShaderStageInfo.pName = "main";

   VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
   fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
   fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
   fragShaderStageInfo.module = fragShaderModule;
   fragShaderStageInfo.pName = "main";

   return {{device, vertShaderStageInfo}, {device, fragShaderStageInfo}};
}

} // namespace shady::render::vulkan
