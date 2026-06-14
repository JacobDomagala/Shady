#include "model.hpp"
#include "render/texture.hpp"
#include "render/vertex.hpp"
#include "trace/logger.hpp"
#include "utils/assert.hpp"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <glm/gtc/quaternion.hpp>
#include <limits>
#include <stdexcept>
#include <string_view>
#include <utility>
#include <vector>

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYGLTF_NOEXCEPTION
#include <tiny_gltf.h>

namespace shady::scene {

void
Model::LoadModel(const std::string& file)
{
   tinygltf::Model model;
   tinygltf::TinyGLTF loader;
   std::string err, warn;

   auto extension = std::filesystem::path(file).extension().string();
   std::ranges::transform(extension, extension.begin(),
                          [](unsigned char c) { return static_cast< char >(std::tolower(c)); });

   bool ok = false;
   if (extension == ".glb")
   {
      ok = loader.LoadBinaryFromFile(&model, &err, &warn, file);
   }
   else if (extension == ".gltf")
   {
      ok = loader.LoadASCIIFromFile(&model, &err, &warn, file);
   }
   else
   {
      utils::Assert(false, fmt::format("Unsupported model format: {}", extension));
   }

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
   auto checkedInt = [](size_t idx, std::string_view name) -> int {
      utils::Assert(idx <= static_cast< size_t >((std::numeric_limits< int >::max)()),
                    fmt::format("glTF {} index does not fit int: {}", name, idx));
      return static_cast< int >(idx);
   };

   struct MaterialGPU
   {
      const render::Texture* baseColor{};
      const render::Texture* normal{};
      const render::Texture* mr{};
      glm::vec4 baseColorFactor = glm::vec4(1.0F);
      float metallicFactor = 1.0F;
      float roughnessFactor = 1.0F;
      float normalScale = 1.0F;
      bool alphaBlend = false;
   };

   std::vector< MaterialGPU > gpuMaterials(model.materials.size());

   auto texOf = [&](int idx, render::TextureType type) -> const render::Texture* {
      if (idx < 0)
         return nullptr;

      const auto texIdx = checkedIndex(idx, model.textures.size(), "texture");
      const auto& tex = model.textures[texIdx];

      const auto imgIdx = checkedIndex(tex.source, model.images.size(), "image");
      const auto& img = model.images[imgIdx];
      utils::Assert(img.bits == 8 && img.pixel_type == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE,
                    fmt::format("glTF image {} must contain 8-bit unsigned pixels", imgIdx));
      utils::Assert(img.component == 4,
                    fmt::format("glTF image {} must decode to RGBA pixels", imgIdx));
      utils::Assert(
         img.width > 0 && img.height > 0,
         fmt::format("glTF image {} has invalid dimensions {}x{}", imgIdx, img.width, img.height));

      const auto width = static_cast< uint32_t >(img.width);
      const auto height = static_cast< uint32_t >(img.height);
      const auto expectedSize =
         static_cast< size_t >(width) * static_cast< size_t >(height) * size_t{4};
      utils::Assert(img.image.size() == expectedSize,
                    fmt::format("glTF image {} contains {} bytes, expected {}", imgIdx,
                                img.image.size(), expectedSize));

      const auto id =
         fmt::format("{}#image_{}#type_{}", file, imgIdx, static_cast< uint32_t >(type));
      render::TextureLibrary::CreateTexture(type, id, img.image.data(), width, height);
      return &render::TextureLibrary::GetTexture(id);
   };

   for (size_t i = 0; i < model.materials.size(); ++i)
   {
      const auto& m = model.materials[i];

      gpuMaterials[i].baseColor =
         texOf(m.pbrMetallicRoughness.baseColorTexture.index, render::TextureType::DIFFUSE_MAP);
      gpuMaterials[i].mr = texOf(m.pbrMetallicRoughness.metallicRoughnessTexture.index,
                                 render::TextureType::METALLIC_ROUGHNESS_MAP);
      gpuMaterials[i].normal = texOf(m.normalTexture.index, render::TextureType::NORMAL_MAP);

      for (size_t channel = 0; channel < m.pbrMetallicRoughness.baseColorFactor.size(); ++channel)
      {
         gpuMaterials[i].baseColorFactor[static_cast< glm::length_t >(channel)] =
            static_cast< float >(m.pbrMetallicRoughness.baseColorFactor[channel]);
      }
      gpuMaterials[i].metallicFactor = static_cast< float >(m.pbrMetallicRoughness.metallicFactor);
      gpuMaterials[i].roughnessFactor =
         static_cast< float >(m.pbrMetallicRoughness.roughnessFactor);
      gpuMaterials[i].normalScale = static_cast< float >(m.normalTexture.scale);
      gpuMaterials[i].alphaBlend = m.alphaMode == "BLEND";
   }

   auto fetch = [&](const tinygltf::Accessor& acc, const tinygltf::Model& m) -> const uint8_t* {
      const auto viewIdx = checkedIndex(acc.bufferView, m.bufferViews.size(), "bufferView");
      const auto& view = m.bufferViews[viewIdx];
      const auto bufferIdx = checkedIndex(view.buffer, m.buffers.size(), "buffer");
      const auto& buffer = m.buffers[bufferIdx];
      // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
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
      // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
      const auto* elemPtr = basePtr + (idx * stride);
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
      // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
      const auto* elemPtr = basePtr + (idx * stride);
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
      // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
      const auto* elemPtr = basePtr + (idx * stride);
      return *reinterpret_cast< const glm::vec4* >(elemPtr);
   };

   auto nodeLocalMat = [](const tinygltf::Node& node) {
      if (node.matrix.size() == 16)
      {
         auto local = glm::mat4(1.0F);
         for (size_t i = 0; i < 16; ++i)
         {
            local[static_cast< int >(i / 4)][static_cast< int >(i % 4)] =
               static_cast< float >(node.matrix[i]);
         }
         return local;
      }

      auto local = glm::mat4(1.0F);
      if (node.translation.size() == 3)
      {
         local = glm::translate(local, glm::vec3(static_cast< float >(node.translation[0]),
                                                 static_cast< float >(node.translation[1]),
                                                 static_cast< float >(node.translation[2])));
      }

      if (node.rotation.size() == 4)
      {
         const glm::quat rotation(
            static_cast< float >(node.rotation[3]), static_cast< float >(node.rotation[0]),
            static_cast< float >(node.rotation[1]), static_cast< float >(node.rotation[2]));
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

      render::MaterialData material{};

      if (prim.material >= 0)
      {
         const auto materialIdx = checkedIndex(prim.material, gpuMaterials.size(), "material");
         const auto& materials = gpuMaterials[materialIdx];
         if (materials.alphaBlend)
         {
            trace::Logger::Warn(
               "Skipping blended primitive in mesh {}: deferred transparency is unsupported",
               meshName);
            return;
         }

         material.textures[0] = materials.baseColor ? materials.baseColor->GetName() : "";
         material.textures[1] = materials.mr ? materials.mr->GetName() : "";
         material.textures[2] = materials.normal ? materials.normal->GetName() : "";
         material.baseColorFactor = materials.baseColorFactor;
         material.metallicFactor = materials.metallicFactor;
         material.roughnessFactor = materials.roughnessFactor;
         material.normalScale = materials.normalScale;
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
      const glm::mat3 tangentMat = glm::mat3(worldMat);
      const float tangentOrientation = glm::determinant(tangentMat) < 0.0F ? -1.0F : 1.0F;

      for (size_t i = 0; i < posAcc.count; ++i)
      {
         render::Vertex v{};

         const auto localPos = readVec3(posAcc, i);
         v.m_position = glm::vec3(worldMat * glm::vec4(localPos, 1.0F));

         if (nrmAcc != nullptr)
         {
            v.m_normal = glm::normalize(normalMat * readVec3(*nrmAcc, i));
         }
         else
         {
            v.m_normal = glm::vec3(0.0F);
         }

         if (uvAcc != nullptr)
         {
            v.m_texCoords = readVec2(*uvAcc, i);
         }
         else
         {
            v.m_texCoords = glm::vec2(0.0F);
         }

         if (tanAcc != nullptr)
         {
            const auto tangent = readVec4(*tanAcc, i);
            v.m_tangent = glm::vec4(glm::normalize(tangentMat * glm::vec3(tangent)),
                                    tangent.w * tangentOrientation);
         }
         else
         {
            v.m_tangent = glm::vec4(0.0F);
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
         const auto componentSize =
            tinygltf::GetComponentSizeInBytes(static_cast< uint32_t >(idxAcc.componentType));
         utils::Assert(componentSize > 0, "Invalid component size for index accessor");

         const auto rawStride = idxAcc.ByteStride(idxView);
         if (rawStride != 0)
         {
            utils::Assert(rawStride > 0, "Invalid byte stride for index accessor");
         }
         const auto stride = rawStride == 0 ? static_cast< size_t >(componentSize)
                                            : static_cast< size_t >(rawStride);

         indices.resize(idxAcc.count);
         for (size_t i = 0; i < idxAcc.count; ++i)
         {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            const auto* elemPtr = idxPtr + (i * stride);
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
         for (size_t i = 0; i < indices.size(); ++i)
         {
            indices[i] = static_cast< uint32_t >(i);
         }
      }

      numVertices_ += static_cast< uint32_t >(vertices.size());
      numIndices_ += static_cast< uint32_t >(indices.size());
      meshes_.emplace_back(meshName, std::move(vertices), std::move(indices), std::move(material));
   };

   std::function< void(int, const glm::mat4&) > processNode = [&](int nodeIndex,
                                                                  const glm::mat4& parentMat) {
      const auto nodeIdx = checkedIndex(nodeIndex, model.nodes.size(), "node");
      const auto& node = model.nodes[nodeIdx];
      const auto worldMat = parentMat * nodeLocalMat(node);

      if (node.mesh >= 0)
      {
         const auto meshIdx = checkedIndex(node.mesh, model.meshes.size(), "mesh");
         const auto& mesh = model.meshes[meshIdx];
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
         processNode(rootNode, glm::mat4(1.0F));
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

      for (size_t i = 0; i < model.nodes.size(); ++i)
      {
         if (!isChild[i])
         {
            processNode(checkedInt(i, "node"), glm::mat4(1.0F));
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
