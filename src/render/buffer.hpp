#pragma once

#include <vector>
#include <vulkan/vulkan.h>


namespace shady::render {

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
   CopyData(const void* data) const;

   void
   CopyDataWithStaging(void* data, size_t dataSize);

   static void
   CopyDataToImageWithStaging(VkImage image, void* data, size_t dataSize,
                              const std::vector< VkBufferImageCopy >& copyRegions);

   void
   Flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) const;

   void
   SetupDescriptor(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

   void
   Destroy();

   [[nodiscard]] VkBuffer&
   GetBuffer();

   [[nodiscard]] void*
   GetMappedMemory();

   [[nodiscard]] VkDescriptorBufferInfo&
   GetDescriptor();

 private:
   void* mappedMemory_ = nullptr;
   bool mapped_ = false;
   VkBuffer buffer_ = {};
   VkDeviceMemory bufferMemory_ = {};
   VkDeviceSize bufferSize_ = {};
   VkDescriptorBufferInfo descriptor_ = {};
};


} // namespace shady::render
