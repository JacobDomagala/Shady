#pragma once

#include "texture.hpp"

namespace shady::render::opengl {

class OpenGLTexture : public Texture
{
 public:
   explicit OpenGLTexture(const std::string& name, TextureType type);
   explicit OpenGLTexture(const glm::ivec2& size, TextureType type);

   ~OpenGLTexture() override;

   void
   Bind(uint32_t slot = 0) const override;

   virtual bool
   operator==(const Texture& other) const override;

 private:
   void
   CreateTexture();

   void
   CreateCubeMap(const std::string& name);
};

} // namespace shady::render::opengl