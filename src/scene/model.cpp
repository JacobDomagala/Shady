#include "model.hpp"
#include "trace/logger.hpp"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

namespace shady::scene {

static render::TextureType
GetShadyTexFromAssimpTex(aiTextureType assimpTex)
{
   switch (assimpTex)
   {
      case aiTextureType_SPECULAR:
         return render::TextureType::SPECULAR_MAP;
      case aiTextureType_NORMALS:
         return render::TextureType::NORMAL_MAP;
      case aiTextureType_DIFFUSE:
      default: {
         return render::TextureType::DIFFUSE_MAP;
      }
   }
}

Model::Model(const std::string& path)
{
   // Read file via ASSIMP
   Assimp::Importer importer;
   auto scene = importer.ReadFile(
      path, aiProcess_GenSmoothNormals | aiProcess_Triangulate | aiProcess_CalcTangentSpace
               | aiProcess_JoinIdenticalVertices | aiProcess_ValidateDataStructure);
   // Check for errors
   if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
   {
      trace::Logger::Fatal("Error loading model: {} \n Error message: {}", path,
                           importer.GetErrorString());
      return;
   }

   trace::Logger::Debug("Loading model: {}", path);

   // Process ASSIMP's root node recursively
   ProcessNode(scene->mRootNode, scene);

   m_name = scene->mRootNode->mName.C_Str();
   trace::Logger::Info("Loaded model: {} numVertices: {} numIndices: {}", m_name, m_numVertices, m_numIndices);
}

void
Model::ScaleModel(const glm::vec3& scale)
{
   m_scaleValue = scale;
   RecalculateModelMat();
}

void
Model::TranslateModel(const glm::vec3& translate)
{
   m_translateValue = translate;
   RecalculateModelMat();
}

void
Model::RotateModel(const glm::vec3& rotate, float angle)
{
   m_rotateAngle = angle;
   m_rotateValue = rotate;
   RecalculateModelMat();
}

void
Model::Draw()
{
   for (auto& mesh : m_meshes)
   {
      mesh.Draw(m_name, m_modelMat, {1.0f, 1.0f, 1.0f, 1.0f});
   }
}

std::vector< Mesh >&
Model::GetMeshes()
{
   return m_meshes;
}

void
Model::ProcessNode(aiNode* node, const aiScene* scene)
{
   for (uint32_t i = 0; i < node->mNumMeshes; i++)
   {
      // The node object only contains indices to index the actual objects in the scene.
      // The scene contains all the data, node is just to keep stuff organized (like relations
      // between nodes).
      auto mesh = scene->mMeshes[node->mMeshes[i]];
      m_meshes.push_back(ProcessMesh(mesh, scene));
   }

   trace::Logger::Debug("Processed node: {}", node->mName.C_Str());

   // After we've processed all of the meshes (if any) we then recursively process each of the
   // children nodes
   for (uint32_t i = 0; i < node->mNumChildren; i++)
   {
      ProcessNode(node->mChildren[i], scene);
   }
}

Mesh
Model::ProcessMesh(aiMesh* mesh, const aiScene* scene)
{
   // Data to fill
   std::vector< render::Vertex > vertices;

   // Walk through each of the mesh's vertices
   for (uint32_t i = 0; i < mesh->mNumVertices; i++)
   {
      render::Vertex vertex;
      glm::vec3 vector;
      // Positions
      vector.x = mesh->mVertices[i].x;
      vector.y = mesh->mVertices[i].y;
      vector.z = mesh->mVertices[i].z;
      vertex.m_position = vector;

      // Normals
      if (mesh->HasNormals())
      {
         vector.x = mesh->mNormals[i].x;
         vector.y = mesh->mNormals[i].y;
         vector.z = mesh->mNormals[i].z;
         vertex.m_normal = vector;
      }

      // Texture Coordinates
      if (mesh->HasTextureCoords(0))
      {
         glm::vec2 vec;
         // A vertex can contain up to 8 different texture coordinates. We thus make the assumption
         // that we won't use models where a vertex can have multiple texture coordinates so we
         // always take the first set (0).
         vec.x = mesh->mTextureCoords[0][i].x;
         vec.y = mesh->mTextureCoords[0][i].y;
         vertex.m_texCoords = vec;
      }
      else
      {
         vertex.m_texCoords = glm::vec2(0.0f, 0.0f);
      }

      if (mesh->HasTangentsAndBitangents())
      {
         // Tangents
         vector.x = mesh->mTangents[i].x;
         vector.y = mesh->mTangents[i].y;
         vector.z = mesh->mTangents[i].z;
         vertex.m_tangent = vector;
      }

      vertices.push_back(vertex);
   }

   std::vector< uint32_t > indices;
   // Now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the
   // corresponding vertex indices.
   for (uint32_t i = 0; i < mesh->mNumFaces; i++)
   {
      aiFace face = mesh->mFaces[i];
      // Retrieve all indices of the face and store them in the indices vector
      for (uint32_t j = 0; j < face.mNumIndices; j++)
      {
         indices.push_back(face.mIndices[j]);
      }
   }

   render::TexturePtrVec textures;

   // Process materials
   if (mesh->mMaterialIndex >= 0)
   {
      aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

      LoadMaterialTextures(material, aiTextureType_DIFFUSE, textures);
      LoadMaterialTextures(material, aiTextureType_SPECULAR, textures);
      LoadMaterialTextures(material, aiTextureType_NORMALS, textures);
   }

   trace::Logger::Debug("Processed mesh: {}", mesh->mName.C_Str());
   m_numVertices += mesh->mNumVertices;
   m_numIndices += static_cast<uint32_t>(indices.size());

   return Mesh(mesh->mName.C_Str(), std::move(vertices), std::move(indices), std::move(textures));
}

void
Model::LoadMaterialTextures(aiMaterial* mat, aiTextureType type, render::TexturePtrVec& textures)
{
   for (uint32_t i = 0; i < mat->GetTextureCount(type); i++)
   {
      aiString str;
      mat->GetTexture(type, i, &str);

      textures.push_back(
         render::TextureLibrary::GetTexture(GetShadyTexFromAssimpTex(type), str.C_Str()));
   }
}

void
Model::RecalculateModelMat()
{
   m_modelMat = glm::translate(glm::mat4(1.0f), m_translateValue)
                * glm::rotate(glm::mat4(1.0f), m_rotateAngle, m_rotateValue)
                * glm::scale(glm::mat4(1.0f), m_scaleValue);
}
} // namespace shady::scene