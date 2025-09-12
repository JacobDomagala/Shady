#include "model.hpp"
#include "render/texture.hpp"
#include "render/vertex.hpp"
#include "trace/logger.hpp"
#include "utils/file_manager.hpp"
#include "utils/assert.hpp"

#include <functional>
#include <numeric>
#include <string_view>
#include <stdexcept>
#include <glm/gtc/quaternion.hpp>

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
   if (!warn.empty())
   {
      trace::Logger::Warn("tinygltf load warning: {}", warn);
   }

   name_ = file;

   auto checkedIndex = [](int idx, size_t containerSize, std::string_view name) -> size_t {
      utils::Assert(idx >= 0, fmt::format("glTF {} index is negative: {}", name, idx));
      const auto converted = static_cast< size_t >(idx);
      utils::Assert(converted < containerSize,
                    fmt::format("glTF {} index out of range: {}", name, idx));
      return converted;
   };

   struct MaterialGPU
   {
      const render::Texture* baseColor{};
      const render::Texture* normal{};
      const render::Texture* mr{};
   };

   std::vector< MaterialGPU > gpuMaterials(model.materials.size());

   auto texOf = [&](int idx, render::TextureType type) -> const render::Texture* {
      if (idx < 0)
         return nullptr;

      const auto texIdx = checkedIndex(idx, model.textures.size(), "texture");
      const auto& tex = model.textures[texIdx];

      const auto imgIdx = checkedIndex(tex.source, model.images.size(), "image");
      const auto& img = model.images[imgIdx];
      std::string id = img.uri.empty() ? ("embed_" + std::to_string(imgIdx)) : img.uri;
      
      render::TextureLibrary::CreateTexture(type, id);
      return &render::TextureLibrary::GetTexture(id);
   };

   for (size_t i = 0; i < model.materials.size(); ++i)
   {
      const auto& m = model.materials[i];
          
      gpuMaterials[i].baseColor =
         texOf(m.pbrMetallicRoughness.baseColorTexture.index, render::TextureType::DIFFUSE_MAP);
      gpuMaterials[i].mr = texOf(m.pbrMetallicRoughness.metallicRoughnessTexture.index, render::TextureType::SPECULAR_MAP);
      gpuMaterials[i].normal = texOf(m.normalTexture.index, render::TextureType::NORMAL_MAP);
   }

   auto fetch = [&](const tinygltf::Accessor& acc, const tinygltf::Model& m) -> const uint8_t* {
      const auto viewIdx = checkedIndex(acc.bufferView, m.bufferViews.size(), "bufferView");
      const auto& view = m.bufferViews[viewIdx];
      const auto bufferIdx = checkedIndex(view.buffer, m.buffers.size(), "buffer");
      const auto& buffer = m.buffers[bufferIdx];
      return buffer.data.data() + view.byteOffset + acc.byteOffset;
   };

   auto readVec2 = [&](const tinygltf::Accessor& acc, size_t idx) -> glm::vec2 {
      utils::Assert(acc.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT
                       && acc.type == TINYGLTF_TYPE_VEC2,
                    "Unsupported accessor format for VEC2");

      const auto viewIdx = checkedIndex(acc.bufferView, model.bufferViews.size(), "bufferView");
      const auto& view = model.bufferViews[viewIdx];
      const auto* basePtr = fetch(acc, model);
      const auto strideSigned = acc.ByteStride(view);
      utils::Assert(strideSigned > 0, "Invalid byte stride for VEC2 accessor");
      const auto stride = static_cast< size_t >(strideSigned);
      const auto* elemPtr = basePtr + idx * stride;
      return *reinterpret_cast< const glm::vec2* >(elemPtr);
   };

   auto readVec3 = [&](const tinygltf::Accessor& acc, size_t idx) -> glm::vec3 {
      utils::Assert(acc.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT
                       && acc.type == TINYGLTF_TYPE_VEC3,
                    "Unsupported accessor format for VEC3");

      const auto viewIdx = checkedIndex(acc.bufferView, model.bufferViews.size(), "bufferView");
      const auto& view = model.bufferViews[viewIdx];
      const auto* basePtr = fetch(acc, model);
      const auto strideSigned = acc.ByteStride(view);
      utils::Assert(strideSigned > 0, "Invalid byte stride for VEC3 accessor");
      const auto stride = static_cast< size_t >(strideSigned);
      const auto* elemPtr = basePtr + idx * stride;
      return *reinterpret_cast< const glm::vec3* >(elemPtr);
   };

   auto readVec4 = [&](const tinygltf::Accessor& acc, size_t idx) -> glm::vec4 {
      utils::Assert(acc.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT
                       && acc.type == TINYGLTF_TYPE_VEC4,
                    "Unsupported accessor format for VEC4");

      const auto viewIdx = checkedIndex(acc.bufferView, model.bufferViews.size(), "bufferView");
      const auto& view = model.bufferViews[viewIdx];
      const auto* basePtr = fetch(acc, model);
      const auto strideSigned = acc.ByteStride(view);
      utils::Assert(strideSigned > 0, "Invalid byte stride for VEC4 accessor");
      const auto stride = static_cast< size_t >(strideSigned);
      const auto* elemPtr = basePtr + idx * stride;
      return *reinterpret_cast< const glm::vec4* >(elemPtr);
   };

   auto nodeLocalMat = [](const tinygltf::Node& node) {
      if (node.matrix.size() == 16)
      {
         glm::mat4 local = glm::mat4(1.0f);
         for (size_t i = 0; i < 16; ++i)
         {
            local[static_cast< int >(i / 4)][static_cast< int >(i % 4)] =
               static_cast< float >(node.matrix[i]);
         }
         return local;
      }

      glm::mat4 local = glm::mat4(1.0f);
      if (node.translation.size() == 3)
      {
         local = glm::translate(local,
                                glm::vec3(static_cast< float >(node.translation[0]),
                                          static_cast< float >(node.translation[1]),
                                          static_cast< float >(node.translation[2])));
      }

      if (node.rotation.size() == 4)
      {
         const glm::quat rotation(static_cast< float >(node.rotation[3]),
                                  static_cast< float >(node.rotation[0]),
                                  static_cast< float >(node.rotation[1]),
                                  static_cast< float >(node.rotation[2]));
         local *= glm::mat4_cast(rotation);
      }

      if (node.scale.size() == 3)
      {
         local = glm::scale(local, glm::vec3(static_cast< float >(node.scale[0]),
                                             static_cast< float >(node.scale[1]),
                                             static_cast< float >(node.scale[2])));
      }

      return local;
   };

   auto processPrimitive = [&](const tinygltf::Primitive& prim, const glm::mat4& worldMat,
                               const std::string& meshName) {
      if (prim.mode != TINYGLTF_MODE_TRIANGLES)
      {
         trace::Logger::Warn("Skipping primitive in mesh {}: only TRIANGLES are supported",
                             meshName);
         return;
      }

      const auto posIt = prim.attributes.find("POSITION");
      if (posIt == prim.attributes.end())
      {
         trace::Logger::Warn("Skipping primitive in mesh {}: no POSITION attribute", meshName);
         return;
      }

      render::TextureMaps texts = {};
      texts[0] = "196.png";
      texts[1] = texts[0];
      texts[2] = texts[0];

      if (prim.material >= 0 && static_cast< size_t >(prim.material) < gpuMaterials.size())
      {
         const auto& materials = gpuMaterials[static_cast< size_t >(prim.material)];
         texts[0] = materials.baseColor ? materials.baseColor->GetName() : texts[0];
         texts[1] = materials.mr ? materials.mr->GetName() : texts[0];
         texts[2] = materials.normal ? materials.normal->GetName() : texts[0];
      }

      const auto posAccessorIdx =
         checkedIndex(posIt->second, model.accessors.size(), "POSITION accessor");
      const auto& posAcc = model.accessors[posAccessorIdx];

      const tinygltf::Accessor* nrmAcc = nullptr;
      const tinygltf::Accessor* uvAcc = nullptr;
      const tinygltf::Accessor* tanAcc = nullptr;

      if (auto it = prim.attributes.find("NORMAL"); it != prim.attributes.end())
      {
         const auto nrmAccessorIdx =
            checkedIndex(it->second, model.accessors.size(), "NORMAL accessor");
         nrmAcc = &model.accessors[nrmAccessorIdx];
      }
      if (auto it = prim.attributes.find("TEXCOORD_0"); it != prim.attributes.end())
      {
         const auto uvAccessorIdx =
            checkedIndex(it->second, model.accessors.size(), "TEXCOORD_0 accessor");
         uvAcc = &model.accessors[uvAccessorIdx];
      }
      if (auto it = prim.attributes.find("TANGENT"); it != prim.attributes.end())
      {
         const auto tanAccessorIdx =
            checkedIndex(it->second, model.accessors.size(), "TANGENT accessor");
         tanAcc = &model.accessors[tanAccessorIdx];
      }

      std::vector< render::Vertex > vertices(posAcc.count);
      const glm::mat3 normalMat = glm::transpose(glm::inverse(glm::mat3(worldMat)));

      for (size_t i = 0; i < posAcc.count; ++i)
      {
         render::Vertex v{};

         const auto localPos = readVec3(posAcc, i);
         v.m_position = glm::vec3(worldMat * glm::vec4(localPos, 1.0f));

         if (nrmAcc != nullptr)
         {
            v.m_normal = glm::normalize(normalMat * readVec3(*nrmAcc, i));
         }
         else
         {
            v.m_normal = glm::vec3(0.0f);
         }

         if (uvAcc != nullptr)
         {
            v.m_texCoords = readVec2(*uvAcc, i);
         }
         else
         {
            v.m_texCoords = glm::vec2(0.0f);
         }

         if (tanAcc != nullptr)
         {
            const auto tangent = readVec4(*tanAcc, i);
            v.m_tangent = glm::normalize(normalMat * glm::vec3(tangent));
         }
         else
         {
            v.m_tangent = glm::vec3(0.0f);
         }

         vertices[i] = v;
      }

      std::vector< uint32_t > indices;
      if (prim.indices >= 0)
      {
         const auto idxAccessorIdx =
            checkedIndex(prim.indices, model.accessors.size(), "indices accessor");
         const auto& idxAcc = model.accessors[idxAccessorIdx];
         const auto idxViewIdx =
            checkedIndex(idxAcc.bufferView, model.bufferViews.size(), "indices bufferView");
         const auto& idxView = model.bufferViews[idxViewIdx];
         const auto* idxPtr = fetch(idxAcc, model);
         const auto componentSize = tinygltf::GetComponentSizeInBytes(
            static_cast< uint32_t >(idxAcc.componentType));
         utils::Assert(componentSize > 0, "Invalid component size for index accessor");

         const auto rawStride = idxAcc.ByteStride(idxView);
         if (rawStride != 0)
         {
            utils::Assert(rawStride > 0, "Invalid byte stride for index accessor");
         }
         const auto stride =
            rawStride == 0 ? static_cast< size_t >(componentSize) : static_cast< size_t >(rawStride);

         indices.resize(idxAcc.count);
         for (size_t i = 0; i < idxAcc.count; ++i)
         {
            const auto* elemPtr = idxPtr + i * stride;
            switch (idxAcc.componentType)
            {
               case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
                  indices[i] = *reinterpret_cast< const uint8_t* >(elemPtr);
                  break;
               case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
                  indices[i] = *reinterpret_cast< const uint16_t* >(elemPtr);
                  break;
               case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
                  indices[i] = *reinterpret_cast< const uint32_t* >(elemPtr);
                  break;
               default:
                  throw std::runtime_error("unsupported index type");
            }
         }
      }
      else
      {
         indices.resize(vertices.size());
         std::iota(indices.begin(), indices.end(), 0);
      }

      numVertices_ += static_cast< uint32_t >(vertices.size());
      numIndices_ += static_cast< uint32_t >(indices.size());
      meshes_.emplace_back(Mesh{meshName, std::move(vertices), std::move(indices), std::move(texts)});
   };

   std::function< void(int, const glm::mat4&) > processNode =
      [&](int nodeIndex, const glm::mat4& parentMat) {
         const auto nodeIdx = checkedIndex(nodeIndex, model.nodes.size(), "node");
         const auto& node = model.nodes[nodeIdx];
         const auto worldMat = parentMat * nodeLocalMat(node);

         if (node.mesh >= 0 && static_cast< size_t >(node.mesh) < model.meshes.size())
         {
            const auto& mesh = model.meshes[static_cast< size_t >(node.mesh)];
            for (const auto& prim : mesh.primitives)
            {
               processPrimitive(prim, worldMat, mesh.name);
            }
            trace::Logger::Debug("Loaded mesh {} from node {}", mesh.name, node.name);
         }

         for (const auto childNode : node.children)
         {
            processNode(childNode, worldMat);
         }
      };

   int sceneIndex = model.defaultScene;
   if (sceneIndex < 0)
   {
      sceneIndex = model.scenes.empty() ? -1 : 0;
   }

   if (sceneIndex >= 0 && static_cast< size_t >(sceneIndex) < model.scenes.size())
   {
      const auto& scene = model.scenes[static_cast< size_t >(sceneIndex)];
      for (const auto rootNode : scene.nodes)
      {
         processNode(rootNode, glm::mat4(1.0f));
      }
   }
   else
   {
      // Fallback for malformed scenes: find root nodes from parent-child relationships.
      std::vector< bool > isChild(model.nodes.size(), false);
      for (const auto& node : model.nodes)
      {
         for (const auto childNode : node.children)
         {
            if (childNode >= 0 && static_cast< size_t >(childNode) < isChild.size())
            {
               isChild[static_cast< size_t >(childNode)] = true;
            }
         }
      }

      for (int i = 0; i < static_cast< int >(model.nodes.size()); ++i)
      {
         if (!isChild[static_cast< size_t >(i)])
         {
            processNode(i, glm::mat4(1.0f));
         }
      }
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
