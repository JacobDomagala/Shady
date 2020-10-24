#include "opengl_texture.hpp"
#include "trace/logger.hpp"
#include "utils/file_manager.hpp"


#include <glad/glad.h>

namespace shady::render::opengl {

OpenGLTexture::OpenGLTexture(TextureType type, const std::string& name) : Texture(type, name)
{
   switch (type)
   {
      case TextureType::DIFFUSE_MAP:
      case TextureType::SPECULAR_MAP:
      case TextureType::NORMAL_MAP: {
         m_imageData = utils::FileManager::ReadTexture(name);
         CreateTexture();
      }
      break;

      case TextureType::CUBE_MAP: {
         CreateCubeMap(name);
      }
      break;
   }
}

OpenGLTexture::OpenGLTexture(TextureType type, const glm::ivec2& size) : Texture(type)
{
   // cast here to avoid overflow warning
   m_imageData.m_bytes = ImageHandleType(
      new uint8_t[static_cast< uint64_t >(size.x) * static_cast< uint64_t >(size.y)]);

   m_imageData.m_size = size;
   m_imageData.m_channels = 4;

   CreateTexture();
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

   trace::Logger::Debug("OpenGL texture loaded = {}", m_name);
}

void
OpenGLTexture::CreateCubeMap(const std::string& name)
{
   std::unordered_map< std::string, GLenum > faces = {
      {name + "_right.jpg", GL_TEXTURE_CUBE_MAP_POSITIVE_X},
      {name + "_left.jpg", GL_TEXTURE_CUBE_MAP_NEGATIVE_X},
      {name + "_top.jpg", GL_TEXTURE_CUBE_MAP_POSITIVE_Y},
      {name + "_bottom.jpg", GL_TEXTURE_CUBE_MAP_NEGATIVE_Y},
      {name + "_back.jpg", GL_TEXTURE_CUBE_MAP_NEGATIVE_Z},
      {name + "_front.jpg", GL_TEXTURE_CUBE_MAP_POSITIVE_Z}};

   glGenTextures(1, &m_textureHandle);
   glBindTexture(GL_TEXTURE_CUBE_MAP, m_textureHandle);

   for (auto& [textureName, glFaceOrientation] : faces)
   {
      auto textureData = utils::FileManager::ReadTexture(textureName);
      glTexImage2D(glFaceOrientation, 0, GL_RGB, textureData.m_size.x, textureData.m_size.y, 0,
                   GL_RGB, GL_UNSIGNED_BYTE, textureData.m_bytes.get());

      trace::Logger::Debug("OpenGL cubemap texture loaded = {}", textureName);
   }

   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
   glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
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