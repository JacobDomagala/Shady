#include "model.hpp"
#include "gltf_loader.hpp"
#include "render/texture.hpp"
#include "trace/logger.hpp"

namespace shady::scene {

void
Model::LoadModel(const std::string& file)
{
   auto asset = LoadGltf(file);
   for (const auto& warning : asset.warnings)
      trace::Logger::Warn("{}: {}", file, warning);
   std::vector< Mesh > meshes;
   uint32_t vertices = 0;
   uint32_t indices = 0;
   for (auto& mesh : asset.meshes)
   {
      for (size_t slot = 0; slot < mesh.images.size(); ++slot)
      {
         if (mesh.images[slot] < 0)
            continue;
         const auto imageIndex = static_cast< size_t >(mesh.images[slot]);
         const auto& image = asset.source.images[imageIndex];
         const auto type = static_cast< render::TextureType >(slot);
         const auto id = fmt::format("{}#image_{}#type_{}", file, imageIndex, slot);
         render::TextureLibrary::CreateTexture(type, id, image.image.data(),
                                               static_cast< uint32_t >(image.width),
                                               static_cast< uint32_t >(image.height));
         mesh.material.textures[slot] = id;
      }
      vertices += static_cast< uint32_t >(mesh.vertices.size());
      indices += static_cast< uint32_t >(mesh.indices.size());
      meshes.emplace_back(mesh.name, std::move(mesh.vertices), std::move(mesh.indices),
                          std::move(mesh.material));
   }
   meshes_ = std::move(meshes);
   numVertices_ = vertices;
   numIndices_ = indices;
   name_ = file;
}

Model::Model(const std::string& path)
{
   trace::Logger::Debug("Loading model: {}", path);

   LoadModel(path);

   trace::Logger::Info("Loaded model: {} numVertices: {} numIndices: {}", name_, numVertices_,
                       numIndices_);
}

void
Model::ScaleModel(const glm::vec3& scale)
{
   for (auto& mesh : meshes_)
   {
      mesh.Scale(scale);
   }
}

void
Model::TranslateModel(const glm::vec3& translate)
{
   for (auto& mesh : meshes_)
   {
      mesh.Translate(translate);
   }
}

void
Model::RotateModel(const glm::vec3& rotate, float angle)
{
   for (auto& mesh : meshes_)
   {
      mesh.Rotate(angle, rotate);
   }
}

void
Model::Submit()
{
   for (auto& mesh : meshes_)
   {
      mesh.Submit();
   }
}

void
Model::Draw()
{
   for (auto& mesh : meshes_)
   {
      mesh.Draw(name_, glm::mat4(1.0F), {1.0F, 1.0F, 1.0F, 1.0F});
   }
}

std::vector< Mesh >&
Model::GetMeshes()
{
   return meshes_;
}

std::unique_ptr< Model >
Model::CreatePlane()
{
   auto model = std::make_unique< Model >();
   model->GetMeshes().push_back({"Plane",
                                 {{
                                     {25.0F, -0.5F, 25.0F},    // Position
                                     {0.0F, 1.0F, 0.0F},       // Normal
                                     {25.0F, 0.0F},            // Texcoord
                                     {50.0F, 0.0F, 0.0F, 1.0F} // Tangent
                                  },
                                  {
                                     {-25.0F, -0.5F, 25.0F},   // Position
                                     {0.0F, 1.0F, 0.0F},       // Normal
                                     {0.0F, 0.0F},             // Texcoord
                                     {50.0F, 0.0F, 0.0F, 1.0F} // Tangent
                                  },
                                  {
                                     {-25.0F, -0.5F, -25.0F},  // Position
                                     {0.0F, 1.0F, 0.0F},       // Normal
                                     {0.0F, 25.0F},            // Texcoord
                                     {50.0F, 0.0F, 0.0F, 1.0F} // Tangent
                                  },
                                  {
                                     {25.0F, -0.5F, -25.0F},   // Position
                                     {0.0F, 1.0F, 0.0F},       // Normal
                                     {25.0F, 25.0F},           // Texcoord
                                     {50.0F, 0.0F, 0.0F, 1.0F} // Tangent
                                  }},
                                 {2, 1, 0, 3, 2, 0}, // Indices
                                 {}});

   return model;
}

} // namespace shady::scene
