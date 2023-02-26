#pragma once

#include "mesh.hpp"
#include "render/types.hpp"

#include <memory>
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
   Submit();

   void
   Draw();

   [[nodiscard]] std::vector< Mesh >&
   GetMeshes();

   [[nodiscard]] static std::unique_ptr< Model >
   CreatePlane();

   //  void
   //  ReloadModel();

 private:
   void
   ProcessNode(aiNode* node, const aiScene* scene);

   [[nodiscard]] Mesh
   ProcessMesh(aiMesh* mesh, const aiScene* scene);

 private:
   std::vector< Mesh > meshes_ = {};
   uint32_t numVertices_ = 0;
   uint32_t numIndices_ = 0;

   std::string name_ = "DefaultName";
};

} // namespace shady::scene
