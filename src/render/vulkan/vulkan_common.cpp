#include "vulkan_common.hpp"
#include "utils/assert.hpp"

namespace shady::render::vulkan {

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

} // namespace shady::render::vulkan
