#include "opengl_texture.hpp"
#include "utils/file_manager.hpp"

#include <glad/glad.h>

namespace shady::render::opengl {

OpenGLTexture::OpenGLTexture(const std::string& name)
{
   m_imageData = utils::FileManager::ReadTexture(name);
   CreateTexture();
}

OpenGLTexture::OpenGLTexture(const glm::ivec2& size)
{
   // cast here to avoid overflow warning
   m_imageData.m_bytes = ImageHandleType(
      new uint8_t[static_cast< uint64_t >(size.x) * static_cast< uint64_t >(size.y)]);

   m_imageData.m_size = size;
   m_imageData.m_channels = 4;
}

OpenGLTexture::~OpenGLTexture()
{
   glDeleteTextures(1, &m_textureHandle);
}

void
OpenGLTexture::CreateTexture()
{
   const auto width = m_imageData.m_size.x;
   const auto height = m_imageData.m_size.y;
   const auto dataFormat = m_imageData.m_channels == 4 ? GL_RGBA : GL_RGB;
   const auto internalFormat = m_imageData.m_channels == 4 ? GL_RGBA8 : GL_RGB8;

   glCreateTextures(GL_TEXTURE_2D, 1, &m_textureHandle);
   glTextureStorage2D(m_textureHandle, 1, internalFormat, width, height);

   glTextureParameteri(m_textureHandle, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glTextureParameteri(m_textureHandle, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

   glTextureParameteri(m_textureHandle, GL_TEXTURE_WRAP_S, GL_REPEAT);
   glTextureParameteri(m_textureHandle, GL_TEXTURE_WRAP_T, GL_REPEAT);

   glTextureSubImage2D(m_textureHandle, 0, 0, 0, width, height, dataFormat, GL_UNSIGNED_BYTE,
                       m_imageData.m_bytes.get());
}

void
OpenGLTexture::Bind(uint32_t slot) const
{
   glBindTextureUnit(slot, m_textureHandle);
}

bool
OpenGLTexture::operator==(const Texture& other) const
{
   return m_textureHandle == other.GetTextureID();
}

} // namespace shady::render::opengl