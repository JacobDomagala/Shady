#include "model.hpp"
#include "render/texture.hpp"
#include "render/vertex.hpp"
#include "trace/logger.hpp"
#include "utils/file_manager.hpp"
#include "utils/assert.hpp"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYGLTF_NOEXCEPTION
#include <tiny_gltf.h>

namespace shady::scene {

// static render::TextureType
// GetShadyTexFromAssimpTex(aiTextureType assimpTex)
// {
//    switch (assimpTex)
//    {
//       case aiTextureType_SPECULAR:
//       case aiTextureType_UNKNOWN:
//          return render::TextureType::SPECULAR_MAP;
//       case aiTextureType_NORMALS:
//          return render::TextureType::NORMAL_MAP;
//       case aiTextureType_DIFFUSE:
//       default: {
//          return render::TextureType::DIFFUSE_MAP;
//       }
//    }
// }

// static void
// LoadMaterialTextures(aiMaterial* mat, aiTextureType type, render::TextureMaps& textures)
// {
//    for (uint32_t i = 0; i < mat->GetTextureCount(type); i++)
//    {
//       aiString str;
//       mat->GetTexture(type, i, &str);

//       const auto texType = GetShadyTexFromAssimpTex(type);
//       render::TextureLibrary::CreateTexture(texType, str.C_Str());
//       textures[static_cast< uint32_t >(texType)] = str.C_Str();
//    }
// }

void
Model::LoadModel(const std::string& file)
{
   tinygltf::Model model;
   tinygltf::TinyGLTF loader;
   std::string err, warn;


   const bool ok = (file.ends_with(".glb") ? loader.LoadBinaryFromFile(&model, &err, &warn, file)
                                           : loader.LoadASCIIFromFile(&model, &err, &warn, file));
   utils::Assert(ok, fmt::format("tinygltf load error: {}\n", err));

   name_ = file;

   std::vector< const render::Texture* > gpuImages(model.images. size());

   for (size_t i = 0; i < model.images.size(); ++i)
   {
      const auto& img = model.images[i];
      const std::string id = img.uri.empty() ? ("embed_" + std::to_string(i)) : img.uri;

      // register in library if not present
      render::TextureLibrary::CreateTexture(render::TextureType::DIFFUSE_MAP, id);

      // store pointer for quick access
      gpuImages[i] = &render::TextureLibrary::GetTexture(id);

      // if image is embedded (uri empty) load from img.image vector
      // ─ your FileManager::ReadTexture takes a path; embed support needs another branch
   }

   struct MaterialGPU
   {
      const render::Texture* baseColor{};
      const render::Texture* normal{};
      const render::Texture* mr{};
   };

   std::vector< MaterialGPU > gpuMaterials(model.materials.size());

   auto texOf = [&](int texIndex) -> const render::Texture* {
      return texIndex >= 0 ? gpuImages[model.textures[texIndex].source] : nullptr;
   };

   for (size_t i = 0; i < model.materials.size(); ++i)
   {
      const auto& m = model.materials[i];
      gpuMaterials[i].baseColor = texOf(m.pbrMetallicRoughness.baseColorTexture.index);
      gpuMaterials[i].mr = texOf(m.pbrMetallicRoughness.metallicRoughnessTexture.index);
      gpuMaterials[i].normal = texOf(m.normalTexture.index);
   }

   auto fetch = [&](const tinygltf::Accessor& acc, const tinygltf::Model& m) -> const uint8_t* {
      const auto& view = m.bufferViews[acc.bufferView];
      const auto& buffer = m.buffers[view.buffer];
      return buffer.data.data() + view.byteOffset + acc.byteOffset;
   };

   for (const auto& mesh : model.meshes)
   {
      for (const auto& prim : mesh.primitives)
      {
         Mesh md;
         auto& materials = gpuMaterials[prim.material];
         render::TextureMaps texts;
         texts[0] = materials.baseColor ? materials.baseColor->GetName() : "196.png";
         texts[1] = materials.mr ? materials.mr->GetName() : texts[0];
         texts[2] = materials.normal ? materials.normal->GetName() : texts[0];

         const auto& posAcc = model.accessors[prim.attributes.at("POSITION")];
         const auto* posPtr = reinterpret_cast< const glm::vec3* >(fetch(posAcc, model));

         const glm::vec3* nrmPtr = nullptr;
         const glm::vec2* uvPtr = nullptr;
         const glm::vec3* tanPtr = nullptr;

         if (auto it = prim.attributes.find("NORMAL"); it != prim.attributes.end())
            nrmPtr =
               reinterpret_cast< const glm::vec3* >(fetch(model.accessors[it->second], model));
         if (auto it = prim.attributes.find("TEXCOORD_0"); it != prim.attributes.end())
            uvPtr = reinterpret_cast< const glm::vec2* >(fetch(model.accessors[it->second], model));
         if (auto it = prim.attributes.find("TANGENT"); it != prim.attributes.end())
            tanPtr =
               reinterpret_cast< const glm::vec3* >(fetch(model.accessors[it->second], model));

         std::vector< render::Vertex > vertices(posAcc.count);

         for (size_t i = 0; i < posAcc.count; ++i)
         {
            render::Vertex v{};
            v.m_position = posPtr[i];
            v.m_normal = nrmPtr ? nrmPtr[i] : glm::vec3(0);
            v.m_texCoords = uvPtr ? uvPtr[i] : glm::vec2(0);
            v.m_tangent = tanPtr ? tanPtr[i] : glm::vec3(0);
            vertices[i] = v;
         }

         const auto& idxAcc = model.accessors[prim.indices];
         const void* idxPtr = fetch(idxAcc, model);

         std::vector< uint32_t > indices(idxAcc.count);
         if (idxAcc.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
            std::transform(reinterpret_cast< const uint16_t* >(idxPtr),
                           reinterpret_cast< const uint16_t* >(idxPtr) + idxAcc.count,
                           indices.begin(), [](uint16_t v) { return uint32_t(v); });
         else if (idxAcc.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)
            std::memcpy(indices.data(), idxPtr, idxAcc.count * sizeof(uint32_t));
         else
            throw std::runtime_error("unsupported index type");

         numVertices_ += vertices.size();
         numIndices_ += static_cast< uint32_t >(indices.size());

         meshes_.emplace_back(
            Mesh{mesh.name, std::move(vertices), std::move(indices), std::move(texts)});
      }

      trace::Logger::Debug("Loaded mesh {}", mesh.name);
   }
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
      mesh.Draw(name_, glm::mat4(1.0f), {1.0f, 1.0f, 1.0f, 1.0f});
   }
}

std::vector< Mesh >&
Model::GetMeshes()
{
   return meshes_;
}

void
Model::ProcessNode(void*, const void*)
{
   // for (uint32_t i = 0; i < node->mNumMeshes; i++)
   // {
   //    // The node object only contains indices to index the actual objects in the scene.
   //    // The scene contains all the data, node is just to keep stuff organized (like relations
   //    // between nodes).
   //    auto* mesh = scene->mMeshes[node->mMeshes[i]];
   //    meshes_.push_back(ProcessMesh(mesh, scene));
   // }

   // trace::Logger::Debug("Processed node: {}", node->mName.C_Str());

   // // After we've processed all of the meshes (if any) we then recursively process each of the
   // // children nodes
   // for (uint32_t i = 0; i < node->mNumChildren; i++)
   // {
   //    ProcessNode(node->mChildren[i], scene);
   // }
}

Mesh
Model::ProcessMesh(void*, const void*)
{
   // Data to fill
   // std::vector< render::Vertex > vertices;

   // // Walk through each of the mesh's vertices
   // for (uint32_t i = 0; i < mesh->mNumVertices; i++)
   // {
   //    render::Vertex vertex{};
   //    glm::vec3 vector{};
   //    // Positions
   //    vector.x = mesh->mVertices[i].x;
   //    vector.y = mesh->mVertices[i].y;
   //    vector.z = mesh->mVertices[i].z;
   //    vertex.m_position = vector;

   //    // Normals
   //    if (mesh->HasNormals())
   //    {
   //       vector.x = mesh->mNormals[i].x;
   //       vector.y = mesh->mNormals[i].y;
   //       vector.z = mesh->mNormals[i].z;
   //       vertex.m_normal = vector;
   //    }

   //    // Texture Coordinates
   //    if (mesh->HasTextureCoords(0))
   //    {
   //       glm::vec2 vec{};
   //       // A vertex can contain up to 8 different texture coordinates. We thus make the
   //       assumption
   //       // that we won't use models where a vertex can have multiple texture coordinates so we
   //       // always take the first set (0).
   //       vec.x = mesh->mTextureCoords[0][i].x;
   //       vec.y = mesh->mTextureCoords[0][i].y;
   //       vertex.m_texCoords = vec;
   //    }
   //    else
   //    {
   //       vertex.m_texCoords = glm::vec2(0.0f, 0.0f);
   //    }

   //    if (mesh->HasTangentsAndBitangents())
   //    {
   //       // Tangents
   //       vector.x = mesh->mTangents[i].x;
   //       vector.y = mesh->mTangents[i].y;
   //       vector.z = mesh->mTangents[i].z;
   //       vertex.m_tangent = vector;
   //    }

   //    vertices.push_back(vertex);
   // }

   // std::vector< uint32_t > indices{};
   // // Now walk through each of the mesh's faces (a face is a mesh its triangle) and retrieve the
   // // corresponding vertex indices.
   // for (uint32_t i = 0; i < mesh->mNumFaces; i++)
   // {
   //    const auto face = mesh->mFaces[i];
   //    // Retrieve all indices of the face and store them in the indices vector
   //    for (uint32_t j = 0; j < face.mNumIndices; j++)
   //    {
   //       indices.push_back(face.mIndices[j]);
   //    }
   // }

   // // render::TexturePtrVec textures;
   // render::TextureMaps textures = {};

   // // Process materials
   // aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
   // LoadMaterialTextures(material, aiTextureType_DIFFUSE, textures);
   // // LoadMaterialTextures(material, aiTextureType_SPECULAR, textures);
   // LoadMaterialTextures(material, aiTextureType_UNKNOWN, textures);
   // LoadMaterialTextures(material, aiTextureType_NORMALS, textures);


   // trace::Logger::Debug("Processed mesh: {}", mesh->mName.C_Str());
   // numVertices_ += mesh->mNumVertices;
   // numIndices_ += static_cast< uint32_t >(indices.size());

   // // NOLINTNEXTLINE
   // return Mesh(mesh->mName.C_Str(), std::move(vertices), std::move(indices),
   // std::move(textures));
   return {};
}

std::unique_ptr< Model >
Model::CreatePlane()
{
   auto model = std::make_unique< Model >();
   model->GetMeshes().push_back({"Plane",
                                 {{
                                     {25.0f, -0.5f, 25.0f}, // Position
                                     {0.0f, 1.0f, 0.0f},    // Normal
                                     {25.0f, 0.0f},         // Texcoord
                                     {50.0f, 0.0f, 0.0f}    // Tangent
                                  },
                                  {
                                     {-25.0f, -0.5f, 25.0f}, // Position
                                     {0.0f, 1.0f, 0.0f},     // Normal
                                     {0.0f, 0.0f},           // Texcoord
                                     {50.0f, 0.0f, 0.0f}     // Tangent
                                  },
                                  {
                                     {-25.0f, -0.5f, -25.0f}, // Position
                                     {0.0f, 1.0f, 0.0f},      // Normal
                                     {0.0f, 25.0f},           // Texcoord
                                     {50.0f, 0.0f, 0.0f}      // Tangent
                                  },
                                  {
                                     {25.0f, -0.5f, -25.0f}, // Position
                                     {0.0f, 1.0f, 0.0f},     // Normal
                                     {25.0f, 25.0f},         // Texcoord
                                     {50.0f, 0.0f, 0.0f}     // Tangent
                                  }},
                                 {2, 1, 0, 3, 2, 0}, // Indices
                                 {}});

   return model;
}

} // namespace shady::scene
