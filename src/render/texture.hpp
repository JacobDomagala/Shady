#pragma once

#include <functional>
#include <glm/glm.hpp>
#include <memory>
#include <unordered_map>

namespace shady::render {

enum class TextureType
{
   DIFFUSE_MAP,
   SPECULAR_MAP,
   NORMAL_MAP
};

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
   virtual ~Texture() = default;

   virtual uint32_t
   GetWidth() const = 0;

   virtual uint32_t
   GetHeight() const = 0;

   virtual uint32_t
   GetTextureID() const = 0;

   virtual void
   Bind(uint32_t slot = 0) const = 0;

   virtual bool
   operator==(const Texture& other) const = 0;

   static std::shared_ptr< Texture >
   Create(const std::string& textureName);

   static std::shared_ptr< Texture >
   Create(const glm::ivec2& size);
};

using TexturePtr = std::shared_ptr< Texture >;
using TexturePtrVec = std::vector< TexturePtr >;

class TextureLibrary
{
 public:
   static TexturePtr
   GetTexture(const std::string& textureName);

   static void
   Clear();

 private:
   static void
   LoadTexture(const std::string& textureName);

 private:
   static inline std::unordered_map< std::string, TexturePtr > s_loadedTextures = {};
};
} // namespace shady::render