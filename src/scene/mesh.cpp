#include "mesh.hpp"
#include "render/renderer_3D.hpp"

namespace shady::scene {

Mesh::Mesh(std::vector< render::Vertex >&& vertices, std::vector< uint32_t >&& indices,
           render::TexturePtrVec&& textures)
{
   m_vertices = std::move(vertices);
   m_indices = std::move(indices);
   m_textures = std::move(textures);
}

void
Mesh::Draw()
{
   render::Renderer3D::DrawMesh(m_vertices, m_indices);
}

} // namespace shady::scene