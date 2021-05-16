#include "common.hpp"
#include "utils/assert.hpp"

namespace shady::render {

uint32_t
FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
   VkPhysicalDeviceMemoryProperties memProperties;
   vkGetPhysicalDeviceMemoryProperties(Data::vk_physicalDevice, &memProperties);

   for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
   {
      if ((typeFilter & (1 << i))
          && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
      {
         return i;
      }
   }

   utils::Assert(false, "failed to find suitable memory type!");
}

VkFormat
FindSupportedFormat(const std::vector< VkFormat >& candidates, VkImageTiling tiling,
                    VkFormatFeatureFlags features)
{
   for (VkFormat format : candidates)
   {
      VkFormatProperties props;
      vkGetPhysicalDeviceFormatProperties(Data::vk_physicalDevice, format, &props);

      if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
      {
         return format;
      }
      else if (tiling == VK_IMAGE_TILING_OPTIMAL
               && (props.optimalTilingFeatures & features) == features)
      {
         return format;
      }
   }

   utils::Assert(false, "failed to find supported format!");
   return {};
}

VkFormat
FindDepthFormat()
{
   return FindSupportedFormat(
      {VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT,
       VK_FORMAT_D16_UNORM_S8_UINT, VK_FORMAT_D16_UNORM},
      VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

} // namespace shady::render
