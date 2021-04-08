#pragma once

#include <vulkan/vulkan.h>

namespace shady::render::vulkan {

class Command
{
 public:
   static VkCommandBuffer
   BeginSingleTimeCommands();

   static void
   EndSingleTimeCommands(VkCommandBuffer commandBuffer);
};

} // namespace shady::render::vulkan
