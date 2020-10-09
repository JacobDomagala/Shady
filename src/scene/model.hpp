#pragma once

#include "mesh.hpp"
#include "render/texture.hpp"

#include <string>
#include <vector>

#include <assimp/material.h>
#include <glm/glm.hpp>

struct aiNode;
struct aiMesh;
struct aiScene;

namespace shady::scene {

class Model
{
 public:
   Model() = default;

   void
   ScaleModel(const glm::vec3& scale);

   void
   TranslateModel(const glm::vec3& translate);

   void
   RotateModel(const glm::vec3& rotate, float angle);

   void
   Draw();

   void
   LoadModel(const std::string& path);

 private:
   void
   ProcessNode(aiNode* node, const aiScene* scene);
   Mesh
   ProcessMesh(aiMesh* mesh, const aiScene* scene);

   void
   LoadMaterialTextures(aiMaterial* mat, aiTextureType type, render::TexturePtrVec& textures);

 private:
   // Model matrix data
   glm::vec3 m_translateValue = {0.0f, 0.0f, 0.0f};
   glm::vec3 m_scaleValue = {1.0f, 1.0f, 1.0f};
   glm::vec3 m_rotateValue = {1.0f, 1.0f, 1.0f};
   float m_rotateAngle = 0.0f;

   std::vector< Mesh > m_meshes = {};
};

} // namespace shady::scene