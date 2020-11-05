#include "opengl_texture.hpp"
#include "trace/logger.hpp"
#include "utils/file_manager.hpp"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <array>

// Undefine leaking 'max' from Windows
#undef max

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
   glDeleteTextures(1, &m_textureID);
}

void
OpenGLTexture::CreateTexture()
{
   const auto width = m_imageData.m_size.x;
   const auto height = m_imageData.m_size.y;

   if (!width || !height || !m_imageData.m_bytes)
   {
      trace::Logger::Debug(
         "OpenGL texture not loaded due to incorrect size! Name:{} Width:{}, Height:{}", m_name,
         width, height);

      return;
   }

   const auto dataFormat = m_imageData.m_channels == 4 ? GL_RGBA : GL_RGB;
   const auto internalFormat = m_imageData.m_channels == 4 ? GL_RGBA8 : GL_RGB8;

   const auto numMips = 1 + glm::floor(glm::log2(static_cast< float >(::glm::max(width, height))));

   glCreateTextures(GL_TEXTURE_2D, 1, &m_textureID);
   glTextureStorage2D(m_textureID, numMips, internalFormat, width, height);

   glTextureParameteri(m_textureID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
   glTextureParameteri(m_textureID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

   glTextureParameteri(m_textureID, GL_TEXTURE_WRAP_S, GL_REPEAT);
   glTextureParameteri(m_textureID, GL_TEXTURE_WRAP_T, GL_REPEAT);

   glTextureSubImage2D(m_textureID, 0, 0, 0, width, height, dataFormat, GL_UNSIGNED_BYTE,
                       m_imageData.m_bytes.get());

   glGenerateTextureMipmap(m_textureID);

   m_textureHandle = glGetTextureHandleARB(m_textureID);

   trace::Logger::Debug("OpenGL texture loaded = {}", m_name);
}

void
OpenGLTexture::CreateCubeMap(const std::string& name)
{
   std::unordered_map< GLuint, std::string > textureFaces = {
      {0, name + "_right.jpg"},  {1, name + "_left.jpg"}, {2, name + "_top.jpg"},
      {3, name + "_bottom.jpg"}, {4, name + "_front.jpg"}, {5, name + "_back.jpg"}};


   // Preload faces to know data format
   std::array< render::Texture::ImageData, 6 > faces;
   for (auto face : {0, 1, 2, 3, 4, 5})
   {
      faces[face] = utils::FileManager::ReadTexture(textureFaces[face]);
   }

   const auto& imageData = faces[0];
   const auto dataFormat = imageData.m_channels == 4 ? GL_RGBA : GL_RGB;
   const auto internalFormat = imageData.m_channels == 4 ? GL_RGBA8 : GL_RGB8;
   const auto width = imageData.m_size.x;
   const auto height = imageData.m_size.y;

   glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_textureID);
   glTextureStorage2D(m_textureID, 1, GL_RGB8, width, height);

   for (auto faceID : {0, 1, 2, 3, 4, 5})
   {
      const auto& textureData = faces[faceID];

      glTextureSubImage3D(m_textureID, 0, 0, 0, faceID, width, height, 1, dataFormat,
                          GL_UNSIGNED_BYTE, textureData.m_bytes.get());
      trace::Logger::Debug("OpenGL cubemap texture loaded = {}", textureFaces[faceID]);
   }

   glTextureParameteri(m_textureID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTextureParameteri(m_textureID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glTextureParameteri(m_textureID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTextureParameteri(m_textureID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   glTextureParameteri(m_textureID, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

void
OpenGLTexture::Bind(uint32_t slot) const
{
   glBindTextureUnit(slot, m_textureID);
}

bool
OpenGLTexture::operator==(const Texture& other) const
{
   return m_textureID == other.GetTextureID();
}

} // namespace shady::render::opengl