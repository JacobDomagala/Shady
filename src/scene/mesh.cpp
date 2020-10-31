#include "mesh.hpp"
#include "render/renderer_3D.hpp"

namespace shady::scene {

Mesh::Mesh(const std::string& name, std::vector< render::Vertex >&& vertices,
           std::vector< uint32_t >&& indices, render::TexturePtrVec&& textures)
   : m_vertices(std::move(vertices)),
     m_indices(std::move(indices)),
     m_textures(std::move(textures)),
     m_name(name)
{
   render::Renderer3D::AddMesh(m_name, m_vertices, m_indices);
}

void
Mesh::AddTexture(const render::TexturePtr& texture)
{
   m_textures.push_back(texture);
}

void
Mesh::Draw(const std::string& modelName, const glm::mat4& modelMat, const glm::vec4& tintColor)
{
   render::Renderer3D::DrawMesh(modelName, modelMat, m_textures, tintColor);
}

} // namespace shady::scene