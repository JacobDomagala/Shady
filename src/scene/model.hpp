#pragma once

#include "mesh.hpp"
#include "render/texture.hpp"

#include <string>
#include <vector>

#include <assimp/material.h>
#include <assimp/postprocess.h>

#include <glm/glm.hpp>

struct aiNode;
struct aiMesh;
struct aiScene;

namespace shady::scene {

enum class LoadFlags
{
   None = 0,
   FlipUV = aiProcess_FlipUVs
};

class Model
{
 public:
   Model() = default;
   explicit Model(const std::string& path, LoadFlags additionalAssimpFlags = LoadFlags::None);

   void
   ScaleModel(const glm::vec3& scale);

   void
   TranslateModel(const glm::vec3& translate);

   void
   RotateModel(const glm::vec3& rotate, float angle);

   void
   Draw();

   std::vector< Mesh >&
   GetMeshes();

   static std::unique_ptr< Model >
   CreatePlane();

   //  void
   //  ReloadModel();

 private:
   void
   ProcessNode(aiNode* node, const aiScene* scene);

   Mesh
   ProcessMesh(aiMesh* mesh, const aiScene* scene);

   void
   LoadMaterialTextures(aiMaterial* mat, aiTextureType type, render::TexturePtrVec& textures);

   void
   RecalculateModelMat();

 private:
   // Model matrix data
   glm::mat4 m_modelMat = glm::mat4(1.0f);
   glm::vec3 m_translateValue = {0.0f, 0.0f, 0.0f};
   glm::vec3 m_scaleValue = {1.0f, 1.0f, 1.0f};
   glm::vec3 m_rotateValue = {1.0f, 1.0f, 1.0f};
   float m_rotateAngle = 0.0f;

   std::vector< Mesh > m_meshes = {};
   uint32_t m_numVertices = 0;
   uint32_t m_numIndices = 0;

   std::string m_name = "DefaultName";
   bool m_flipUV = false;
};

} // namespace shady::scene
