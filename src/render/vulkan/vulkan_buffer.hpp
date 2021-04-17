#pragma once

#include <vulkan/vulkan.h>

namespace shady::render::vulkan {

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
   Map(VkDeviceSize size = VK_WHOLE_SIZE);

   void
   Unmap();

   void
   CopyData(const void* data);

   void
   SetupDescriptor(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

 public:
   void* m_mappedMemory = nullptr;
   bool m_mapped = false;
   VkBuffer m_buffer = {};
   VkDeviceMemory m_bufferMemory = {};
   VkDeviceSize m_bufferSize = {};
   VkDescriptorBufferInfo m_descriptor = {};
};


} // namespace shady::render::vulkan
