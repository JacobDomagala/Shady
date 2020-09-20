#include "shader.hpp"
#include "opengl/opengl_shader.hpp"
#include "renderer.hpp"

namespace shady::render {

/**************************************************************************************************
 ******************************************* SHADER ***********************************************
 *************************************************************************************************/
std::shared_ptr< Shader >
Shader::Create(const std::string& name)
{
   switch (Renderer::GetAPI())
   {
      case RendererAPI::API::None: {
         trace::Logger::Fatal("Shader::Create() -> RendererAPI::None is currently not supported!");
         return nullptr;
      }
      break;

      case RendererAPI::API::OpenGL: {
         return std::make_shared< opengl::OpenGLShader >(name);
      }
      break;
   }

   trace::Logger::Fatal("Shader::Create() -> Unknown RendererAPI!");
   return nullptr;
}

/**************************************************************************************************
 *************************************** SHADER LIBRARY *******************************************
 *************************************************************************************************/
std::shared_ptr< Shader >
ShaderLibrary::GetShader(const std::string& name)
{
   if (s_shaderLibrary.end() == s_shaderLibrary.find(name))
   {
      LoadShader(name);
   }

   return s_shaderLibrary[name];
}

void
ShaderLibrary::Clear()
{
   s_shaderLibrary.clear();
}

void
ShaderLibrary::LoadShader(const std::string& name)
{
   s_shaderLibrary[name] = Shader::Create(name);
}

} // namespace shady::render