#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <unordered_map>


namespace shady::render {

class Shader
{
 public:
   virtual ~Shader() = default;

   virtual void
   Bind() const = 0;
   virtual void
   Unbind() const = 0;
   virtual void
   SetInt(const std::string& name, int value) = 0;
   virtual void
   SetIntArray(const std::string& name, int* values, uint32_t count) = 0;
   virtual void
   SetFloat(const std::string& name, float value) = 0;
   virtual void
   SetFloat3(const std::string& name, const glm::vec3& value) = 0;
   virtual void
   SetFloat4(const std::string& name, const glm::vec4& value) = 0;
   virtual void
   SetMat4(const std::string& name, const glm::mat4& value) = 0;
   virtual void
   SetMat4Array(const std::string& name, const glm::mat4* matrices, uint32_t count) = 0;

   virtual const std::string&
   GetName() const = 0;

   static std::shared_ptr< Shader >
   Create(const std::string& name);
};

class ShaderLibrary
{
 public:
   static std::shared_ptr< Shader >
   GetShader(const std::string& name);

   static void
   Clear();

 private:
   static void
   LoadShader(const std::string& name);

 private:
   static inline std::unordered_map< std::string, std::shared_ptr< Shader > > s_shaderLibrary = {};
};
} // namespace shady::render