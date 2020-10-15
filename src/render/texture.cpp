#include "texture.hpp"
#include "helpers.hpp"
#include "opengl/opengl_texture.hpp"
#include "renderer.hpp"
#include "trace/logger.hpp"


namespace shady::render {

/**************************************************************************************************
 ****************************************** TEXTURE ***********************************************
 *************************************************************************************************/

Texture::Texture(TextureType type) : m_type(type)
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

TextureHandleType
Texture::GetTextureID() const
{
   return m_textureHandle;
}

TextureType
Texture::GetType() const
{
   return m_type;
}

template < typename... Args >
TexturePtr
Texture::Create(const Args&... args)
{
   return CreateSharedWrapper< opengl::OpenGLTexture, Texture >(args...);
}

template TexturePtr
Texture::Create< std::string >(const std::string& textureName);

template TexturePtr
Texture::Create< glm::ivec2 >(const glm::ivec2& size);

/**************************************************************************************************
 *************************************** TEXTURE LIBRARY ******************************************
 *************************************************************************************************/
TexturePtr
TextureLibrary::GetTexture(const std::string& textureName, TextureType type)
{
   if (s_loadedTextures.find(textureName) == s_loadedTextures.end())
   {
      trace::Logger::Info("Texture: {} not found in library. Loading it", textureName);
      LoadTexture(textureName, type);
   }

   return s_loadedTextures[textureName];
}

void
TextureLibrary::Clear()
{
   s_loadedTextures.clear();
}

void
TextureLibrary::LoadTexture(const std::string& textureName, TextureType type)
{
   s_loadedTextures[textureName] = Texture::Create(textureName, type);
}

} // namespace shady::render