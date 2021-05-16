#include "mesh.hpp"

#include "renderer.hpp"

namespace shady::scene {

Mesh::Mesh(const std::string& name, std::vector< render::Vertex >&& vertices,
           std::vector< uint32_t >&& indices, render::TextureMaps&& textures)
   : m_vertices(std::move(vertices)),
     m_indices(std::move(indices)),
     m_textures(std::move(textures)),
     m_name(name)
{
   // render::Renderer3D::AddMesh(m_name, m_vertices, m_indices);
//   render::Renderer::MeshLoaded(m_vertices, m_indices, m_textures, m_modelMat);
}

// void
// Mesh::AddTexture(const render::TexturePtr& texture)
//{
//   //m_textures.push_back(texture);
//}

void
Mesh::Draw(const std::string& /*modelName*/, const glm::mat4& modelMat, const glm::vec4& tintColor)
{
   // render::Renderer3D::DrawMesh(m_name, modelMat, m_textures, tintColor);
   render::Renderer::MeshLoaded(m_vertices, m_indices, m_textures, m_modelMat);
}

void
Mesh::Scale(const glm::vec3& scale)
{
   m_scaleMat = glm::scale(glm::mat4(1.0f), scale);
   RebuildModelMat();
}

void
Mesh::Rotate(float roateVal, const glm::vec3& axis)
{
   m_rotateMat = glm::rotate(glm::mat4(1.0f), roateVal, axis);
   RebuildModelMat();
}

void
Mesh::Translate(const glm::vec3& translateVal)
{
   m_translateMat = glm::translate(glm::mat4(1.0f), translateVal);
   RebuildModelMat();
}

void
Mesh::RebuildModelMat()
{
   m_modelMat = m_translateMat * m_scaleMat * m_rotateMat;
}

} // namespace shady::scene
