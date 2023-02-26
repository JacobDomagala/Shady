#pragma once

#include "types.hpp"
#include "vertex.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <vector>

namespace shady::scene {

class Mesh
{
 public:
   Mesh(const std::string& name, std::vector< render::Vertex >&& vertices,
        std::vector< uint32_t >&& indices, render::TextureMaps&& textures);

   /*void
   AddTexture(const render::TexturePtr& texture);*/

   void
   Submit();

   void
   Draw(const std::string& modelName, const glm::mat4& modelMat, const glm::vec4& tintColor);

   void
   Scale(const glm::vec3& scale);

   void
   Rotate(float roateVal, const glm::vec3& axis);

   void
   Translate(const glm::vec3& translateVal);

 private:
   void
   RebuildModelMat();

 private:
   glm::mat4 modelMat_ = glm::mat4(1.0f);

   glm::mat4 translateMat_ = glm::mat4(1.0f);
   glm::mat4 rotateMat_ = glm::mat4(1.0f);
   glm::mat4 scaleMat_ = glm::mat4(1.0f);

   std::vector< render::Vertex > vertices_ = {};
   std::vector< uint32_t > indices_ = {};
   // render::TexturePtrVec m_textures = {};
   render::TextureMaps textures_;
   std::string name_ = "dummyMeshName";
};

} // namespace shady::scene
