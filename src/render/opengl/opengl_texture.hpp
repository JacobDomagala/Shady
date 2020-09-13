#pragma once

#include "texture.hpp"

namespace shady::render::opengl {

class OpenGLTexture : public Texture
{
 public:
   explicit OpenGLTexture(const std::string& name);
   explicit OpenGLTexture(const glm::ivec2& size);

   ~OpenGLTexture() override;

   uint32_t
   GetWidth() const override;

   uint32_t
   GetHeight() const override;

   uint32_t
   GetTextureID() const override;

   void
   Bind(uint32_t slot = 0) const override;

   virtual bool
   operator==(const Texture& other) const override;

 private:
   void
   CreateTexture();

 private:
   uint32_t m_textureHandle;
   ImageData m_imageData;
};
} // namespace shady::render::opengl