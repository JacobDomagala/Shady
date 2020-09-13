#pragma once

#include <memory>
#include <unordered_map>

namespace shady::render {

class Texture
{
};

class TextureLibrary
{
 public:
   std::shared_ptr< Texture >
   GetTexture(const std::string& textureName);

 private:
   static inline std::unordered_map< std::string, std::shared_ptr< Texture > > s_textures = {};
};
} // namespace shady::render