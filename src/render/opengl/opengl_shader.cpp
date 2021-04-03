#include "opengl_shader.hpp"
#include "trace/logger.hpp"
#include "utils/assert.hpp"
#include "utils/file_manager.hpp"

#include <array>
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

namespace shady::render::opengl {

template < typename FunctionPtr, typename... Args >
void
SetUniformLocation(const OpenGLShader& program, const std::string& name, FunctionPtr&& ptr,
                   Args&&... args)
{
   const auto location = glGetUniformLocation(program.GetID(), name.c_str());

   if (location < 0)
   {
      trace::Logger::Warn("OpenGL shader location not found! Location name = {} for shader = {}!",
                          name, program.GetName());
   }
   else
   {
      ptr(location, std::forward< Args >(args)...);
   }
}

OpenGLShader::OpenGLShader(const std::string& name) : m_name(name)
{
   Compile(utils::FileManager::ReadTextFile(utils::FileManager::SHADERS_DIR / name
                                        / fmt::format("{}.vs.glsl", name)),
           utils::FileManager::ReadTextFile(utils::FileManager::SHADERS_DIR / name
                                        / fmt::format("{}.fs.glsl", name)),
           utils::FileManager::ReadTextFile(utils::FileManager::SHADERS_DIR / name
                                        / fmt::format("{}.gs.glsl", name)));
}

OpenGLShader::~OpenGLShader()
{
   glDeleteProgram(m_shaderID);
}

uint32_t
OpenGLShader::GetID() const
{
   return m_shaderID;
}

void
OpenGLShader::Compile(const std::string& vertexShader, const std::string& fragmentShader,
                      const std::string& geometryShader)
{
   const auto vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
   const auto fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
   const auto geometryShaderID = glCreateShader(GL_GEOMETRY_SHADER);

   const GLchar* shaderSource = vertexShader.c_str();
   glShaderSource(vertexShaderID, 1, &shaderSource, NULL);

   shaderSource = fragmentShader.c_str();
   glShaderSource(fragmentShaderID, 1, &shaderSource, NULL);

   glCompileShader(vertexShaderID);
   CheckCompileStatus(GL_VERTEX_SHADER, vertexShaderID);
   glCompileShader(fragmentShaderID);
   CheckCompileStatus(GL_FRAGMENT_SHADER, fragmentShaderID);

   m_shaderID = glCreateProgram();
   glAttachShader(m_shaderID, vertexShaderID);
   glAttachShader(m_shaderID, fragmentShaderID);

   if (!geometryShader.empty())
   {
      trace::Logger::Debug("Sources for geometry shader found!");

      shaderSource = geometryShader.c_str();
      glShaderSource(geometryShaderID, 1, &shaderSource, NULL);
      glCompileShader(geometryShaderID);
      CheckCompileStatus(GL_GEOMETRY_SHADER, geometryShaderID);

      glAttachShader(m_shaderID, geometryShaderID);
   }

   glLinkProgram(m_shaderID);
   CheckLinkStatus(m_shaderID);

   glUseProgram(m_shaderID);

   glDeleteShader(vertexShaderID);
   glDeleteShader(fragmentShaderID);
   glDeleteShader(geometryShaderID);
}

void
OpenGLShader::CheckCompileStatus(GLuint type, GLuint programID)
{
   GLint isCompleted = 0;
   glGetShaderiv(programID, GL_COMPILE_STATUS, &isCompleted);
   if (isCompleted == GL_FALSE)
   {
      GLint maxLength = 0;
      glGetShaderiv(programID, GL_INFO_LOG_LENGTH, &maxLength);

      std::string log(static_cast< std::string::size_type >(maxLength), '\0');
      glGetShaderInfoLog(programID, maxLength, &maxLength, &log[0]);

      trace::Logger::Fatal(std::move(log));
   }
   else
   {
      trace::Logger::Debug("OpenGL {} Shader: {} compiled! ",
                           type == GL_VERTEX_SHADER
                              ? "Vertex"
                              : (type == GL_FRAGMENT_SHADER ? "Fragment" : "Geometry"),
                           m_name);
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

      std::string log(static_cast< std::string::size_type >(maxLength), '\0');
      glGetProgramInfoLog(programID, maxLength, &maxLength, &log[0]);

      trace::Logger::Fatal(std::move(log));
   }
   else
   {
      trace::Logger::Debug("OpenGL Shader program: {} linked! ", m_name);
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
OpenGLShader::SetInt(const std::string& name, int32_t value)
{
   UploadUniformInt(name, value);
}

void
OpenGLShader::SetUint(const std::string& name, uint32_t value)
{
   UploadUniformUnsignedInt(name, value);
}

void
OpenGLShader::SetIntArray(const std::string& name, int* values, GLsizei count)
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
   UploadUniformMat4(name, &value);
}

void
OpenGLShader::SetMat4Array(const std::string& name, const glm::mat4* matrices, GLsizei count)
{
   UploadUniformMat4(name, matrices, count);
}

void
OpenGLShader::UploadUniformInt(const std::string& name, int value)
{
   SetUniformLocation(*this, name, glUniform1i, value);
}

void
OpenGLShader::UploadUniformUnsignedInt(const std::string& name, uint32_t value)
{
   SetUniformLocation(*this, name, glUniform1ui, value);
}

void
OpenGLShader::UploadUniformIntArray(const std::string& name, int* values, GLsizei count)
{
   SetUniformLocation(*this, name, glUniform1iv, count, values);
}

void
OpenGLShader::UploadUniformFloat(const std::string& name, float value)
{
   SetUniformLocation(*this, name, glUniform1f, value);
}

void
OpenGLShader::UploadUniformFloat2(const std::string& name, const glm::vec2& value)
{
   SetUniformLocation(*this, name, glUniform2f, value.x, value.y);
}

void
OpenGLShader::UploadUniformFloat3(const std::string& name, const glm::vec3& value)
{
   SetUniformLocation(*this, name, glUniform3f, value.x, value.y, value.z);
}

void
OpenGLShader::UploadUniformFloat4(const std::string& name, const glm::vec4& value)
{
   SetUniformLocation(*this, name, glUniform4f, value.x, value.y, value.z, value.w);
}

void
OpenGLShader::UploadUniformMat3(const std::string& name, const glm::mat3& matrix)
{
   SetUniformLocation(*this, name, glUniformMatrix3fv, 1, static_cast< GLboolean >(GL_FALSE),
                      glm::value_ptr(matrix));
}

void
OpenGLShader::UploadUniformMat4(const std::string& name, const glm::mat4* matrices, GLsizei count)
{
   SetUniformLocation(*this, name, glUniformMatrix4fv, count, static_cast< GLboolean >(GL_FALSE),
                      reinterpret_cast< const GLfloat* >(matrices));
}

} // namespace shady::render::opengl
