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
Mesh::Draw(const glm::vec3& translateVal, const glm::vec3& scaleVal, const glm::vec3 rotateAxis,
           float rotateVal, const glm::vec4& tintColor)
{
   render::Renderer3D::DrawMesh(m_vertices, m_indices, translateVal, scaleVal, rotateAxis,
                                rotateVal, m_textures, tintColor);
}

} // namespace shady::scene