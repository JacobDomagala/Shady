#include "vulkan_texture.hpp"
#include "trace/logger.hpp"
#include "utils/assert.hpp"
#include "utils/file_manager.hpp"
#include "vulkan_buffer.hpp"
#include "vulkan_command.hpp"
#include "vulkan_common.hpp"


#include <string_view>

namespace shady::render::vulkan {

/**************************************************************************************************
 ****************************************** TEXTURE ***********************************************
 *************************************************************************************************/

void Texture::Destroy()
{
   vkDestroyImage(Data::vk_device, m_textureImage, nullptr);
   vkFreeMemory(Data::vk_device, m_textureImageMemory, nullptr);
}

Texture::Texture(TextureType type, std::string_view textureName)
{
   CreateTextureImage(type, textureName);
}

void
Texture::CreateTextureImage(TextureType type, std::string_view textureName)
{
   m_type = type;
   auto textureData = utils::FileManager::ReadTexture(textureName);
   m_width = textureData.m_size.x;
   m_height = textureData.m_size.y;

   m_format = VK_FORMAT_R8G8B8A8_SRGB;

   VkDeviceSize imageSize = m_width * m_height * 4;

   VkBuffer stagingBuffer;
   VkDeviceMemory stagingBufferMemory;
   Buffer::CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                        stagingBuffer, stagingBufferMemory);

   void* data;
   utils::Assert(vkMapMemory(Data::vk_device, stagingBufferMemory, 0, imageSize, 0, &data)
                    == VK_SUCCESS,
                 "Failed to map memory");
   memcpy(data, textureData.m_bytes.get(), static_cast< size_t >(imageSize));
   vkUnmapMemory(Data::vk_device, stagingBufferMemory);

   CreateImage(VK_IMAGE_TILING_OPTIMAL,
               VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

   TransitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
   CopyBufferToImage(stagingBuffer);
   TransitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                         VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

   vkDestroyBuffer(Data::vk_device, stagingBuffer, nullptr);
   vkFreeMemory(Data::vk_device, stagingBufferMemory, nullptr);
}

void
Texture::CreateImage(VkImageTiling tiling, VkImageUsageFlags usage,
                     VkMemoryPropertyFlags properties)
{
   VkImageCreateInfo imageInfo{};
   imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
   imageInfo.imageType = VK_IMAGE_TYPE_2D;
   imageInfo.extent.width = m_width;
   imageInfo.extent.height = m_height;
   imageInfo.extent.depth = 1;
   imageInfo.mipLevels = 1;
   imageInfo.arrayLayers = 1;
   imageInfo.format = m_format;
   imageInfo.tiling = tiling;
   imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
   imageInfo.usage = usage;
   imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
   imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

   utils::Assert(vkCreateImage(Data::vk_device, &imageInfo, nullptr, &m_textureImage) == VK_SUCCESS,
                 "failed to create image!");

   VkMemoryRequirements memRequirements;
   vkGetImageMemoryRequirements(Data::vk_device, m_textureImage, &memRequirements);

   VkMemoryAllocateInfo allocInfo{};
   allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
   allocInfo.allocationSize = memRequirements.size;
   allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

   utils::Assert(vkAllocateMemory(Data::vk_device, &allocInfo, nullptr, &m_textureImageMemory)
                    == VK_SUCCESS,
                 "failed to allocate image memory!");

   vkBindImageMemory(Data::vk_device, m_textureImage, m_textureImageMemory, 0);
}


void
Texture::CopyBufferToImage(VkBuffer buffer)
{
   VkCommandBuffer commandBuffer = Command::BeginSingleTimeCommands();

   VkBufferImageCopy region{};
   region.bufferOffset = 0;
   region.bufferRowLength = 0;
   region.bufferImageHeight = 0;
   region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
   region.imageSubresource.mipLevel = 0;
   region.imageSubresource.baseArrayLayer = 0;
   region.imageSubresource.layerCount = 1;
   region.imageOffset = {0, 0, 0};
   region.imageExtent = {m_width, m_height, 1};

   vkCmdCopyBufferToImage(commandBuffer, buffer, m_textureImage,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

   Command::EndSingleTimeCommands(commandBuffer);
}

void
Texture::TransitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout)
{
   VkCommandBuffer commandBuffer = Command::BeginSingleTimeCommands();

   VkImageMemoryBarrier barrier{};
   barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
   barrier.oldLayout = oldLayout;
   barrier.newLayout = newLayout;
   barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
   barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
   barrier.image = m_textureImage;
   barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
   barrier.subresourceRange.baseMipLevel = 0;
   barrier.subresourceRange.levelCount = 1;
   barrier.subresourceRange.baseArrayLayer = 0;
   barrier.subresourceRange.layerCount = 1;

   VkPipelineStageFlags sourceStage;
   VkPipelineStageFlags destinationStage;

   if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
   {
      barrier.srcAccessMask = 0;
      barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

      sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
      destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
   }
   else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
            && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
   {
      barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
      barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

      sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
      destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
   }
   else
   {
      throw std::invalid_argument("unsupported layout transition!");
   }

   vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1,
                        &barrier);

   Command::EndSingleTimeCommands(commandBuffer);
}


/**************************************************************************************************
 *************************************** TEXTURE LIBRARY ******************************************
 *************************************************************************************************/
const Texture&
TextureLibrary::GetTexture(TextureType type, const std::string& textureName)
{
   if (s_loadedTextures.find(textureName) == s_loadedTextures.end())
   {
      trace::Logger::Debug("Texture: {} not found in library. Loading it", textureName);
      LoadTexture(type, textureName);
   }

   return s_loadedTextures[textureName];
}

void
TextureLibrary::Clear()
{
   s_loadedTextures.clear();
}

void
TextureLibrary::LoadTexture(TextureType type, std::string_view textureName)
{
   s_loadedTextures[std::string{textureName}] = {type, textureName};
}

} // namespace shady::render::vulkan
