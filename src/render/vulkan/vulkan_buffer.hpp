#pragma once

#include <vulkan/vulkan.h>

namespace shady::render::vulkan {

static constexpr VkDeviceSize WHOLE_BUFFER = -1;

class Buffer
{
 public:
   static void
   AllocateImageMemory(VkImage image, VkDeviceMemory& bufferMemory,
                       VkMemoryPropertyFlags properties);

   static void
   AllocateBufferMemory(VkBuffer buffer, VkDeviceMemory& bufferMemory,
                        VkMemoryPropertyFlags properties);

   static Buffer
   CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);

   static void
   CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                VkBuffer& buffer, VkDeviceMemory& bufferMemory);

   static void
   CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

   void
   Map(VkDeviceSize size = WHOLE_BUFFER);

   void
   Unmap();

   void
   CopyData(const void* data);

 public:
   void* m_mappedMemory = nullptr;
   bool m_mapped = false;
   VkBuffer m_buffer = {};
   VkDeviceMemory m_bufferMemory = {};
   VkDeviceSize m_bufferSize = {};
};


} // namespace shady::render::vulkan
