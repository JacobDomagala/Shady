#include "vulkan_buffer.hpp"
#include "utils/assert.hpp"
#include "vulkan_command.hpp"
#include "vulkan_common.hpp"

#include <fmt/format.h>

namespace shady::render::vulkan {

void
Buffer::Map(VkDeviceSize size)
{
   vkMapMemory(Data::vk_device, m_bufferMemory, 0, size, 0, &m_mappedMemory);
   m_mapped = true;
}

void
Buffer::Unmap()
{
   utils::Assert(m_mapped, "Buffer is not mapped!");

   vkUnmapMemory(Data::vk_device, m_bufferMemory);
   m_mapped = false;
   m_mappedMemory = nullptr;
}

void
Buffer::CopyData(const void* data)
{
   utils::Assert(m_mapped, "Buffer is not mapped!");
   memcpy(m_mappedMemory, data, m_bufferSize);
}

void
Buffer::SetupDescriptor(VkDeviceSize size, VkDeviceSize offset)
{
   m_descriptor.offset = offset;
   m_descriptor.buffer = m_buffer;
   m_descriptor.range = m_bufferSize;
}

static void
AllocateMemory(VkMemoryRequirements memReq, VkDeviceMemory& bufferMemory,
               VkMemoryPropertyFlags properties)
{
   VkMemoryAllocateInfo allocInfo{};
   allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
   allocInfo.allocationSize = memReq.size;
   allocInfo.memoryTypeIndex = FindMemoryType(memReq.memoryTypeBits, properties);

   VK_CHECK(vkAllocateMemory(Data::vk_device, &allocInfo, nullptr, &bufferMemory),
            "failed to allocate buffer memory!");
}

void
Buffer::AllocateImageMemory(VkImage image, VkDeviceMemory& bufferMemory,
                            VkMemoryPropertyFlags properties)
{
   VkMemoryRequirements memRequirements;
   vkGetImageMemoryRequirements(Data::vk_device, image, &memRequirements);

   AllocateMemory(memRequirements, bufferMemory, properties);
}

void
Buffer::AllocateBufferMemory(VkBuffer buffer, VkDeviceMemory& bufferMemory,
                             VkMemoryPropertyFlags properties)
{
   VkMemoryRequirements memRequirements;
   vkGetBufferMemoryRequirements(Data::vk_device, buffer, &memRequirements);

   AllocateMemory(memRequirements, bufferMemory, properties);
}

Buffer
Buffer::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
{
   Buffer newBuffer;
   newBuffer.m_bufferSize = size;
   CreateBuffer(size, usage, properties, newBuffer.m_buffer, newBuffer.m_bufferMemory);
   newBuffer.SetupDescriptor();

   return newBuffer;
}

void
Buffer::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                     VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
   VkBufferCreateInfo bufferInfo{};
   bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
   bufferInfo.size = size;
   bufferInfo.usage = usage;
   bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

   VK_CHECK(vkCreateBuffer(Data::vk_device, &bufferInfo, nullptr, &buffer),
            "failed to create buffer!");

   AllocateBufferMemory(buffer, bufferMemory, properties);

   vkBindBufferMemory(Data::vk_device, buffer, bufferMemory, 0);
}

void
Buffer::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
   VkCommandBuffer commandBuffer = Command::BeginSingleTimeCommands();

   VkBufferCopy copyRegion{};
   copyRegion.size = size;
   vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

   Command::EndSingleTimeCommands(commandBuffer);
}

void
Buffer::Flush(VkDeviceSize size, VkDeviceSize offset)
{
   VkMappedMemoryRange mappedRange = {};
   mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
   mappedRange.memory = m_bufferMemory;
   mappedRange.offset = offset;
   mappedRange.size = size;
   VK_CHECK(vkFlushMappedMemoryRanges(Data::vk_device, 1, &mappedRange), "");
}

void
Buffer::Destroy()
{
   if (m_buffer)
   {
      vkDestroyBuffer(Data::vk_device, m_buffer, nullptr);
   }
   if (m_bufferMemory)
   {
      vkFreeMemory(Data::vk_device, m_bufferMemory, nullptr);
   }
}

} // namespace shady::render::vulkan
