#include "mesh.hpp"

#include "renderer.hpp"

namespace shady::scene {

//NOLINTNEXTLINE
Mesh::Mesh(const std::string& name, std::vector< render::Vertex >&& vertices,
           std::vector< uint32_t >&& indices, render::TextureMaps&& textures)
   : vertices_(std::move(vertices)),
     indices_(std::move(indices)),
     textures_(std::move(textures)),
     name_(name)
{
}

// void
// Mesh::AddTexture(const render::TexturePtr& texture)
//{
//   //textures_.push_back(texture);
//}

void
Mesh::Submit()
{
   render::Renderer::MeshLoaded(vertices_, indices_, textures_, modelMat_);
}

void
Mesh::Draw(const std::string& /*modelName*/, const glm::mat4& /*modelMat*/,
           const glm::vec4& /*tintColor*/)
{
   // render::Renderer3D::DrawMesh(name_, modelMat, textures_, tintColor);
   render::Renderer::MeshLoaded(vertices_, indices_, textures_, modelMat_);
}

void
Mesh::Scale(const glm::vec3& scale)
{
   scaleMat_ = glm::scale(glm::mat4(1.0f), scale);
   RebuildModelMat();
}

void
Mesh::Rotate(float roateVal, const glm::vec3& axis)
{
   rotateMat_ = glm::rotate(glm::mat4(1.0f), roateVal, axis);
   RebuildModelMat();
}

void
Mesh::Translate(const glm::vec3& translateVal)
{
   translateMat_ = glm::translate(glm::mat4(1.0f), translateVal);
   RebuildModelMat();
}

void
Mesh::RebuildModelMat()
{
   modelMat_ = translateMat_ * scaleMat_ * rotateMat_;
}

} // namespace shady::scene
