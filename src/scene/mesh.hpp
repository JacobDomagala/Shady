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
   glm::mat4 m_modelMat = glm::mat4(1.0f);

   glm::mat4 m_translateMat = glm::mat4(1.0f);
   glm::mat4 m_rotateMat = glm::mat4(1.0f);
   glm::mat4 m_scaleMat = glm::mat4(1.0f);

   std::vector< render::Vertex > m_vertices = {};
   std::vector< uint32_t > m_indices = {};
   // render::TexturePtrVec m_textures = {};
   render::TextureMaps m_textures;
   std::string m_name = "dummyMeshName";
};

} // namespace shady::scene
