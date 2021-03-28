#include "shader.hpp"
#include "opengl/opengl_shader.hpp"
#include "renderer.hpp"
#include "trace/logger.hpp"
#include "helpers.hpp"

namespace shady::render {

/**************************************************************************************************
 ******************************************* SHADER ***********************************************
 *************************************************************************************************/
std::shared_ptr< Shader >
Shader::Create(const std::string& name)
{
   return CreateSharedWrapper<opengl::OpenGLShader, Shader>(name);
}

/**************************************************************************************************
 *************************************** SHADER LIBRARY *******************************************
 *************************************************************************************************/
std::shared_ptr< Shader >
ShaderLibrary::GetShader(const std::string& name)
{
   if (s_shaderLibrary.end() == s_shaderLibrary.find(name))
   {
      trace::Logger::Debug("Shader: {} not found in library. Loading it.", name);
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