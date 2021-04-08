#pragma once

#include <vulkan/vulkan.h>

namespace shady::render::vulkan {

class Buffer {
  public:

   static void
   CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                VkBuffer& buffer, VkDeviceMemory& bufferMemory);

   static void
   CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
};


} // namespace shady::render::vulkan
