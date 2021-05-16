#pragma once

#include <vulkan/vulkan.h>

namespace shady::render {

class Command
{
 public:
   static VkCommandBuffer
   BeginSingleTimeCommands();

   static void
   EndSingleTimeCommands(VkCommandBuffer commandBuffer);
};

} // namespace shady::render
