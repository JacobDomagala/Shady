#pragma once

#include "render/texture.hpp"
#include "render/vertex.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace shady::scene {

class Mesh
{
 public:
   Mesh(std::vector< render::Vertex >&& vertices, std::vector< uint32_t >&& indices,
        render::TexturePtrVec&& textures);

   void
   AddTexture(const render::TexturePtr& texture);

   void
   Draw(const std::string& modelName, const glm::vec3& translateVal, const glm::vec3& scaleVal,
        const glm::vec3 rotateAxis, float rotateVal, const glm::vec4& tintColor);

 private:
   std::vector< render::Vertex > m_vertices = {};
   std::vector< uint32_t > m_indices = {};
   render::TexturePtrVec m_textures = {};
};

} // namespace shady::scene