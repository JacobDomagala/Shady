#include "vulkan_buffer.hpp"
#include "utils/assert.hpp"
#include "vulkan_command.hpp"
#include "vulkan_common.hpp"


#include <fmt/format.h>

namespace shady::render::vulkan {

void
Buffer::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                     VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
   VkBufferCreateInfo bufferInfo{};
   bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
   bufferInfo.size = size;
   bufferInfo.usage = usage;
   bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

   if (vkCreateBuffer(Data::vk_device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
   {
      throw std::runtime_error("failed to create buffer!");
   }

   VkMemoryRequirements memRequirements;
   vkGetBufferMemoryRequirements(Data::vk_device, buffer, &memRequirements);

   VkMemoryAllocateInfo allocInfo{};
   allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
   allocInfo.allocationSize = memRequirements.size;
   allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

   if (vkAllocateMemory(Data::vk_device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
   {
      throw std::runtime_error("failed to allocate buffer memory!");
   }

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

} // namespace shady::render::vulkan
