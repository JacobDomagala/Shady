#pragma once

#include "vulkan_texture.hpp"

#include <unordered_map>
#include <vulkan/vulkan.h>

namespace shady::render::vulkan {

enum class TextureType
{
   DIFFUSE_MAP,
   SPECULAR_MAP,
   NORMAL_MAP,
   CUBE_MAP
};

class Texture
{
 public:
   Texture(TextureType type, std::string_view textureName);

   Texture() = default;

   void Destroy();

   void
   CreateTextureImage(TextureType type, std::string_view textureName);

   void
   CreateImage(VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties);

 private:
   void
   TransitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout);
   void
   CopyBufferToImage(VkBuffer buffer);

 private:
   TextureType m_type = {};
   VkImage m_textureImage = {};
   VkDeviceMemory m_textureImageMemory = {};
   VkFormat m_format = {};
   int32_t m_channels = {};
   uint32_t m_width = {};
   uint32_t m_height = {};
   std::string m_name = "default_texture_name";
};

class TextureLibrary
{
 public:
   static const Texture&
   GetTexture(TextureType type, const std::string& textureName);

   static void
   Clear();

 private:
   static void
   LoadTexture(TextureType type, std::string_view textureName);

 private:
   static inline std::unordered_map< std::string, Texture > s_loadedTextures = {};
};

} // namespace shady::render::vulkan
