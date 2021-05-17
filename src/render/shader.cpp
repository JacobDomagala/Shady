#include "shader.hpp"
#include "common.hpp"
#include "trace/logger.hpp"
#include "utils/assert.hpp"
#include "utils/file_manager.hpp"

namespace shady::render {

static VkShaderModule
CreateShaderModule(VkDevice device, std::vector< char >&& shaderByteCode)
{
   VkShaderModuleCreateInfo createInfo{};
   createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
   createInfo.codeSize = shaderByteCode.size();
   createInfo.pCode = reinterpret_cast< const uint32_t* >(shaderByteCode.data());

   VkShaderModule shaderModule;
   VK_CHECK(vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule),
            "Failed to create shader module!");

   return shaderModule;
}


ShaderInfoWrapper
Shader::LoadShader(std::string_view shader, VkShaderStageFlagBits stage)
{
   VkShaderModule shaderModule = CreateShaderModule(
      Data::vk_device,
      utils::FileManager::ReadBinaryFile(utils::FileManager::SHADERS_DIR / shader));

   VkPipelineShaderStageCreateInfo shaderStageInfo{};
   shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
   shaderStageInfo.stage = stage;
   shaderStageInfo.module = shaderModule;
   shaderStageInfo.pName = "main";

   return {Data::vk_device, shaderStageInfo};
}

std::pair< VertexShaderInfo, FragmentShaderInfo >
Shader::CreateShader(VkDevice device, std::string_view vertex, std::string_view fragment)
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

} // namespace shady::render
