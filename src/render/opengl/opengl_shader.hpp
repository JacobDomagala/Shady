#pragma once

#include "shader.hpp"

#include <glad/glad.h>

namespace shady::render::opengl {

class OpenGLShader : public Shader
{
 public:
   explicit OpenGLShader(const std::string& name);

   ~OpenGLShader() override;

   void
   Bind() const override;
   void
   Unbind() const override;

   const std::string&
   GetName() const override;

   uint32_t
   GetID() const;

   void
   SetInt(const std::string& name, int32_t value) override;
   void
   SetUint(const std::string& name, uint32_t value) override;

   void
   SetIntArray(const std::string& name, int* values, GLsizei count) override;
   void
   SetFloat(const std::string& name, float value) override;
   void
   SetFloat3(const std::string& name, const glm::vec3& value) override;
   void
   SetFloat4(const std::string& name, const glm::vec4& value) override;
   void
   SetMat4(const std::string& name, const glm::mat4& value) override;
   void
   SetMat4Array(const std::string& name, const glm::mat4* matrices, GLsizei count) override;

   void
   UploadUniformInt(const std::string& name, int value);
   void
   UploadUniformUnsignedInt(const std::string& name, uint32_t value);

   void
   UploadUniformIntArray(const std::string& name, int* values, GLsizei count);

   void
   UploadUniformFloat(const std::string& name, float value);
   void
   UploadUniformFloat2(const std::string& name, const glm::vec2& value);
   void
   UploadUniformFloat3(const std::string& name, const glm::vec3& value);
   void
   UploadUniformFloat4(const std::string& name, const glm::vec4& value);

   void
   UploadUniformMat3(const std::string& name, const glm::mat3& matrix);
   void
   UploadUniformMat4(const std::string& name, const glm::mat4* matrices, GLsizei count = 1);

 private:
   void
   Compile(const std::string& vertexShader, const std::string& fragmentShader,
           const std::string& geometryShader);

   void
   CheckCompileStatus(GLuint type, GLuint programID);

   void
   CheckLinkStatus(GLuint programID);

 private:
   uint32_t m_shaderID;
   std::string m_name;
};

} // namespace shady::render::opengl
