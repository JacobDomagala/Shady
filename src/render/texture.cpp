#include "texture.hpp"
#include "helpers.hpp"
#include "opengl/opengl_texture.hpp"
#include "renderer.hpp"
#include "trace/logger.hpp"


namespace shady::render {

/**************************************************************************************************
 ****************************************** TEXTURE ***********************************************
 *************************************************************************************************/

Texture::Texture(TextureType type, const std::string& name) : m_name(name), m_type(type)
{
}

uint32_t
Texture::GetWidth() const
{
   return m_imageData.m_size.x;
}

uint32_t
Texture::GetHeight() const
{
   return m_imageData.m_size.y;
}

TextureIDType
Texture::GetTextureID() const
{
   return m_textureID;
}

TextureHandleType
Texture::GetTextureHandle() const
{
   return m_textureHandle;
}

TextureType
Texture::GetType() const
{
   return m_type;
}

std::string
Texture::GetName() const
{
   return m_name;
}

TexturePtr
Texture::Create(TextureType type, const std::string& textureName)
{
   return CreateSharedWrapper< opengl::OpenGLTexture, Texture >(type, textureName);
}

TexturePtr
Texture::Create( TextureType type, const glm::ivec2& size)
{
   return CreateSharedWrapper< opengl::OpenGLTexture, Texture >(type, size);
}

/**************************************************************************************************
 *************************************** TEXTURE LIBRARY ******************************************
 *************************************************************************************************/
TexturePtr
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
TextureLibrary::LoadTexture(TextureType type, const std::string& textureName)
{
   s_loadedTextures[textureName] = Texture::Create(type, textureName);
}

} // namespace shady::render
