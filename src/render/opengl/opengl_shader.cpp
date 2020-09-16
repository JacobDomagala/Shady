#include "opengl_shader.hpp"
#include "trace/logger.hpp"
#include "utils/assert.hpp"
#include "utils/file_manager.hpp"

#include <array>
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

namespace shady::render::opengl {

static GLenum
ShaderTypeFromString(const std::string& type)
{
   if (type == "vertex")
   {
      return GL_VERTEX_SHADER;
   }

   if (type == "fragment")
   {
      return GL_FRAGMENT_SHADER;
   }

   utils::Assert(false, "Unknown shader type!");

   return 0;
}

OpenGLShader::OpenGLShader(const std::string& name) : m_name(name)
{
   Compile(utils::FileManager::ReadFile(utils::FileManager::SHADERS_DIR / name
                                        / fmt::format("{}.vs,glsl", name)),
           utils::FileManager::ReadFile(utils::FileManager::SHADERS_DIR / name
                                        / fmt::format("{}.fs,glsl", name)));
}

OpenGLShader::~OpenGLShader()
{
   glDeleteProgram(m_shaderID);
}

void
OpenGLShader::Compile(const std::string& vertexShader, const std::string& fragmentShader)
{
   auto vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
   auto fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

   const GLchar* shaderSource = vertexShader.c_str();
   glShaderSource(vertexShaderID, 1, &shaderSource, NULL);

   shaderSource = fragmentShader.c_str();
   glShaderSource(fragmentShaderID, 1, &shaderSource, NULL);

   glCompileShader(vertexShaderID);
   CheckCompileStatus(vertexShaderID);
   glCompileShader(fragmentShaderID);
   CheckCompileStatus(fragmentShaderID);

   m_shaderID = glCreateProgram();
   glAttachShader(m_shaderID, vertexShaderID);
   glAttachShader(m_shaderID, fragmentShaderID);
   glLinkProgram(m_shaderID);
   CheckLinkStatus(m_shaderID);

   glUseProgram(m_shaderID);

   glDeleteShader(vertexShaderID);
   glDeleteShader(fragmentShaderID);
}

void
OpenGLShader::CheckCompileStatus(GLuint shaderID)
{
   GLint isCompleted = 0;
   glGetShaderiv(shaderID, GL_COMPILE_STATUS, &isCompleted);
   if (isCompleted == GL_FALSE)
   {
      GLint maxLength = 0;
      glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &maxLength);

      std::string log(maxLength, 0);
      glGetShaderInfoLog(shaderID, maxLength, &maxLength, &log[0]);

      trace::Logger::Fatal(std::move(log));
   }
}

void
OpenGLShader::CheckLinkStatus(GLuint programID)
{
   GLint isLinked = 0;
   glGetProgramiv(programID, GL_LINK_STATUS, &isLinked);
   if (isLinked == GL_FALSE)
   {
      GLint maxLength = 0;
      glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &maxLength);

      std::string log(maxLength, 0);
      glGetProgramInfoLog(programID, maxLength, &maxLength, &log[0]);

      trace::Logger::Fatal(std::move(log));
   }
}

void
OpenGLShader::Bind() const
{
   glUseProgram(m_shaderID);
}

void
OpenGLShader::Unbind() const
{
   glUseProgram(0);
}

const std::string&
OpenGLShader::GetName() const
{
   return m_name;
}

void
OpenGLShader::SetInt(const std::string& name, int value)
{
   UploadUniformInt(name, value);
}

void
OpenGLShader::SetIntArray(const std::string& name, int* values, uint32_t count)
{
   UploadUniformIntArray(name, values, count);
}

void
OpenGLShader::SetFloat(const std::string& name, float value)
{
   UploadUniformFloat(name, value);
}

void
OpenGLShader::SetFloat3(const std::string& name, const glm::vec3& value)
{
   UploadUniformFloat3(name, value);
}

void
OpenGLShader::SetFloat4(const std::string& name, const glm::vec4& value)
{
   UploadUniformFloat4(name, value);
}

void
OpenGLShader::SetMat4(const std::string& name, const glm::mat4& value)
{
   UploadUniformMat4(name, value);
}

void
OpenGLShader::UploadUniformInt(const std::string& name, int value)
{
   GLint location = glGetUniformLocation(m_shaderID, name.c_str());
   glUniform1i(location, value);
}

void
OpenGLShader::UploadUniformIntArray(const std::string& name, int* values, uint32_t count)
{
   GLint location = glGetUniformLocation(m_shaderID, name.c_str());
   glUniform1iv(location, count, values);
}

void
OpenGLShader::UploadUniformFloat(const std::string& name, float value)
{
   GLint location = glGetUniformLocation(m_shaderID, name.c_str());
   glUniform1f(location, value);
}

void
OpenGLShader::UploadUniformFloat2(const std::string& name, const glm::vec2& value)
{
   GLint location = glGetUniformLocation(m_shaderID, name.c_str());
   glUniform2f(location, value.x, value.y);
}

void
OpenGLShader::UploadUniformFloat3(const std::string& name, const glm::vec3& value)
{
   GLint location = glGetUniformLocation(m_shaderID, name.c_str());
   glUniform3f(location, value.x, value.y, value.z);
}

void
OpenGLShader::UploadUniformFloat4(const std::string& name, const glm::vec4& value)
{
   GLint location = glGetUniformLocation(m_shaderID, name.c_str());
   glUniform4f(location, value.x, value.y, value.z, value.w);
}

void
OpenGLShader::UploadUniformMat3(const std::string& name, const glm::mat3& matrix)
{
   GLint location = glGetUniformLocation(m_shaderID, name.c_str());
   glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
}

void
OpenGLShader::UploadUniformMat4(const std::string& name, const glm::mat4& matrix)
{
   GLint location = glGetUniformLocation(m_shaderID, name.c_str());
   glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
}

} // namespace shady::render::opengl
