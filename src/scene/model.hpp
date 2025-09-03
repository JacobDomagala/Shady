#pragma once

#include "mesh.hpp"
#include "render/types.hpp"

#include <memory>
#include <string>
#include <vector>

#include <glm/glm.hpp>


namespace shady::scene {


class Model
{
 public:
   Model() = default;
   explicit Model(const std::string& path);

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
   ProcessNode(void* node, const void* scene);

   [[nodiscard]] Mesh
   ProcessMesh(void* mesh, const void* scene);

 private:
   std::vector< Mesh > meshes_ = {};
   uint32_t numVertices_ = 0;
   uint32_t numIndices_ = 0;

   std::string name_ = "DefaultName";
};

} // namespace shady::scene
