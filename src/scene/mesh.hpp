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
   Draw();

 private:
   std::vector< render::Vertex > m_vertices = {};
   std::vector< uint32_t > m_indices = {};
   render::TexturePtrVec m_textures = {};
};

} // namespace shady::scene