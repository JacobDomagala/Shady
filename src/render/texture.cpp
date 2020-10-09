#include "texture.hpp"
#include "opengl/opengl_texture.hpp"
#include "renderer.hpp"

namespace shady::render {

/**************************************************************************************************
 ****************************************** TEXTURE ***********************************************
 *************************************************************************************************/

TextureType
Texture::GetType() const
{
   return m_type;
}

std::shared_ptr< Texture >
Texture::Create(const std::string& textureName)
{
   switch (Renderer::GetAPI())
   {
      case RendererAPI::API::None: {
         trace::Logger::Fatal("Texture::Create() -> RendererAPI::None is currently not supported!");
         return nullptr;
      }
      break;

      case RendererAPI::API::OpenGL: {
         return std::make_shared< opengl::OpenGLTexture >(textureName);
      }
      break;
   }

   trace::Logger::Fatal("Texture::Create() -> Unknown RendererAPI!");
   return nullptr;
}

std::shared_ptr< Texture >
Texture::Create(const glm::ivec2& size)
{
   switch (Renderer::GetAPI())
   {
      case RendererAPI::API::None: {
         trace::Logger::Fatal("Texture::Create() -> RendererAPI::None is currently not supported!");
         return nullptr;
      }
      break;

      case RendererAPI::API::OpenGL: {
         return std::make_shared< opengl::OpenGLTexture >(size);
      }
      break;
   }

   trace::Logger::Fatal("Texture::Create() -> Unknown RendererAPI!");
   return nullptr;
}

/**************************************************************************************************
 *************************************** TEXTURE LIBRARY ******************************************
 *************************************************************************************************/
std::shared_ptr< Texture >
TextureLibrary::GetTexture(const std::string& textureName)
{
   if (s_loadedTextures.find(textureName) == s_loadedTextures.end())
   {
      trace::Logger::Info("Texture: {} not found in library. Loading it", textureName);
      LoadTexture(textureName);
   }

   return s_loadedTextures[textureName];
}

void
TextureLibrary::Clear()
{
   s_loadedTextures.clear();
}

void
TextureLibrary::LoadTexture(const std::string& textureName)
{
   s_loadedTextures[textureName] = Texture::Create(textureName);
}

} // namespace shady::render