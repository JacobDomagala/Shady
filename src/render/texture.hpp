#pragma once

#include <functional>
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <unordered_map>

namespace shady::render {

enum class TextureType
{
   DIFFUSE_MAP,
   SPECULAR_MAP,
   NORMAL_MAP,
   CUBE_MAP
};

class Texture;

using TextureHandleType = uint32_t;
using TexturePtr = std::shared_ptr< Texture >;
using TexturePtrVec = std::vector< TexturePtr >;

class Texture
{
 public:
   using ImageHandleType = std::unique_ptr< uint8_t[], std::function< void(uint8_t*) > >;

   struct ImageData
   {
      ImageHandleType m_bytes;
      glm::ivec2 m_size;
      int32_t m_channels;
   };

 public:
   Texture(TextureType type = TextureType::DIFFUSE_MAP, const std::string& name = "defaultName");
   virtual ~Texture() = default;

   virtual void
   Bind(uint32_t slot = 0) const = 0;

   virtual bool
   operator==(const Texture& other) const = 0;

   uint32_t
   GetWidth() const;

   uint32_t
   GetHeight() const;

   TextureHandleType
   GetTextureID() const;

   TextureType
   GetType() const;

   std::string
   GetName() const;

   static TexturePtr
   Create(TextureType type, const std::string& textureName);

   static TexturePtr
   Create(TextureType type, const glm::ivec2& size);

 protected:
   TextureType m_type;
   ImageData m_imageData;
   TextureHandleType m_textureHandle;
   std::string m_name;
};

class TextureLibrary
{
 public:
   static TexturePtr
   GetTexture(TextureType type, const std::string& textureName);

   static void
   Clear();

 private:
   static void
   LoadTexture(TextureType type, const std::string& textureName);

 private:
   static inline std::unordered_map< std::string, TexturePtr > s_loadedTextures = {};
};

} // namespace shady::render