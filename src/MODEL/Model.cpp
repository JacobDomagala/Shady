#include "Model.h"

#include <assimp/postprocess.h>

Model::Model()
   : translateValue(0.0, 0.0, 0.0),
     rotateValue(1.0, 1.0, 1.0),
     scaleValue(1.0, 1.0, 1.0),
     rotateAngle(0.0)
{
}
void
Model::ScaleModel(vec3 scale)
{
   scaleValue = scale;
}

void
Model::TranslateModel(vec3 translate)
{
   translateValue = translate;
}

void
Model::RotateModel(vec3 rotate, float angle)
{
   rotateAngle = angle;
   rotateValue = rotate;
}

void
Model::LoadModelFromFile(char* path)
{
   LoadModel(path);
}

void
Model::Draw(shady::app::Window* window, Camera camera, Light* lights, Shader shader)
{
   shader.UseProgram();

   glm::vec3 lightPos = lights->position;
   glm::vec3 camPos = camera.position;

   // projectionMatrix = window->projectionMatrix;
   viewMatrix = camera.viewMatrix;
   modelMatrix = glm::translate(translateValue) * glm::rotate(rotateAngle, rotateValue)
                 * glm::scale(scaleValue);

   //glUniformMatrix4fv(glGetUniformLocation(shader.programID, "projectionMatrix"), 1, GL_FALSE,
   //                   glm::value_ptr(projectionMatrix));
   //glUniformMatrix4fv(glGetUniformLocation(shader.programID, "viewMatrix"), 1, GL_FALSE,
   //                   glm::value_ptr(viewMatrix));
   //glUniformMatrix4fv(glGetUniformLocation(shader.programID, "modelMatrix"), 1, GL_FALSE,
   //                   glm::value_ptr(modelMatrix));
   //glUniform3fv(glGetUniformLocation(shader.programID, "vLightPosition"), 1, &lightPos[0]);
   //glUniform3fv(glGetUniformLocation(shader.programID, "vCameraPosition"), 1, &camPos[0]);


   //for (GLuint i = 0; i < meshes.size(); i++)
   //{
   //   meshes[i].Draw(shader.programID);
   //}
}

void
Model::LoadModel(string path)
{
   // Read file via ASSIMP
   Assimp::Importer importer;
   const aiScene* scene =
      importer.ReadFile(path, aiProcess_GenSmoothNormals | aiProcess_Triangulate | aiProcess_FlipUVs
                                 | aiProcess_CalcTangentSpace);
   // Check for errors
   if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
   {
      std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
      return;
   }
   // Retrieve the directory path of the filepath
   directory = path.substr(0, path.find_last_of('/'));

   // Process ASSIMP's root node recursively
   ProcessNode(scene->mRootNode, scene);
}
void
Model::ProcessNode(aiNode* node, const aiScene* scene)
{
   // Process each mesh located at the current node
   //for (GLuint i = 0; i < node->mNumMeshes; i++)
   //{
   //   // The node object only contains indices to index the actual objects in the scene.
   //   // The scene contains all the data, node is just to keep stuff organized (like relations
   //   // between nodes).
   //   aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
   //   meshes.push_back(ProcessMesh(mesh, scene));
   //}
   //// After we've processed all of the meshes (if any) we then recursively process each of the
   //// children nodes
   //for (GLuint i = 0; i < node->mNumChildren; i++)
   //{
   //   ProcessNode(node->mChildren[i], scene);
   //}
}

Mesh
Model::ProcessMesh(aiMesh* mesh, const aiScene* scene)
{
   // Data to fill
   vector< Vertex > vertices;
   vector< uint32_t > indices;
   //vector< Texture > textures;

   //// Walk through each of the mesh's vertices
   //for (GLuint i = 0; i < mesh->mNumVertices; i++)
   //{
   //   Vertex vertex;
   //   vec3 vector;
   //   // Positions
   //   vector.x = mesh->mVertices[i].x;
   //   vector.y = mesh->mVertices[i].y;
   //   vector.z = mesh->mVertices[i].z;
   //   vertex.position = vector;
   //   // Normals
   //   if (mesh->HasNormals())
   //   {
   //      vector.x = mesh->mNormals[i].x;
   //      vector.y = mesh->mNormals[i].y;
   //      vector.z = mesh->mNormals[i].z;
   //      vertex.normal = vector;
   //   }

   //   // Texture Coordinates
   //   if (mesh->HasTextureCoords(0))
   //   {
   //      vec2 vec;
   //      // A vertex can contain up to 8 different texture coordinates. We thus make the assumption
   //      // that we won't use models where a vertex can have multiple texture coordinates so we
   //      // always take the first set (0).
   //      vec.x = mesh->mTextureCoords[0][i].x;
   //      vec.y = mesh->mTextureCoords[0][i].y;
   //      vertex.texCoords = vec;
   //   }
   //   else
   //      vertex.texCoords = vec2(0.0f, 0.0f);

   //   // Tangents
   //   vector.x = mesh->mTangents[i].x;
   //   vector.y = mesh->mTangents[i].y;
   //   vector.z = mesh->mTangents[i].z;
   //   vertex.tangent = vector;


   //   vertices.push_back(vertex);
   //}
   //// Now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the
   //// corresponding vertex indices.
   //for (GLuint i = 0; i < mesh->mNumFaces; i++)
   //{
   //   aiFace face = mesh->mFaces[i];
   //   // Retrieve all indices of the face and store them in the indices vector
   //   for (GLuint j = 0; j < face.mNumIndices; j++)
   //   {
   //      indices.push_back(face.mIndices[j]);
   //   }
   //}
   //// Process materials
   //if (mesh->mMaterialIndex >= 0)
   //{
   //   aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
   //   // 1. Diffuse maps
   //   vector< Texture > diffuseMaps =
   //      LoadMaterialTextures(material, aiTextureType_DIFFUSE, "diffuse_map");
   //   textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
   //   // 2. Specular maps
   //   vector< Texture > specularMaps =
   //      LoadMaterialTextures(material, aiTextureType_SPECULAR, "specular_map");
   //   textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
   //   // 3. Normal maps
   //   vector< Texture > normalMaps =
   //      LoadMaterialTextures(material, aiTextureType_HEIGHT, "normal_map");
   //   textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
   //}

   // Return a mesh object created from the extracted mesh data
   //return Mesh(&vertices, &indices, &textures);
   return {};
}

//vector< Texture >
//Model::LoadMaterialTextures(aiMaterial* mat, aiTextureType type, char* typeName)
//{
//   vector< Texture > textures;
//   //for (GLuint i = 0; i < mat->GetTextureCount(type); i++)
//   //{
//   //   aiString str;
//   //   mat->GetTexture(type, i, &str);
//   //   // Check if texture was loaded before and if so, continue to next iteration: skip loading a
//   //   // new texture
//   //   GLboolean alreadyLoaded = false;
//   //   for (GLuint j = 0; j < textures_loaded.size(); j++)
//   //   {
//   //      if (textures_loaded[j].path == str)
//   //      {
//   //         textures.push_back(textures_loaded[j]);
//   //         alreadyLoaded = true;
//   //         break;
//   //      }
//   //   }
//   //   if (!alreadyLoaded)
//   //   {
//   //      // If texture hasn't been loaded already, load it
//   //      Texture texture;
//   //      texture.LoadTexture(str.C_Str(), typeName, directory);
//   //      texture.path = str;
//   //      textures.push_back(texture);
//   //      textures_loaded.push_back(texture);
//   //   }
//   //}
//   return textures;
//}

void
Model::CreateCube(int size)
{
   //Mesh cube(NULL, NULL, 0);
   Mesh cube;

   Vertex verticies[] = {vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f), vec2(1.0f, 1.0f), vec3(),
                         vec3(size, 0.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f), vec2(0.0f, 1.0f), vec3(),
                         vec3(size, -size, 0.0f), vec3(0.0f, 0.0f, -1.0f), vec2(0.0f, 0.0f), vec3(),
                         vec3(0.0f, -size, 0.0f), vec3(0.0f, 0.0f, -1.0f), vec2(1.0f, 0.0f), vec3(),

                         vec3(size, 0.0f, 0.0f), vec3(1.0f, 0.0f, 0.0f), vec2(1.0f, 1.0f), vec3(),
                         vec3(size, 0.0f, size), vec3(1.0f, 0.0f, 0.0f), vec2(0.0f, 1.0f), vec3(),
                         vec3(size, -size, size), vec3(1.0f, 0.0f, 0.0f), vec2(0.0f, 0.0f), vec3(),
                         vec3(size, -size, 0.0f), vec3(1.0f, 0.0f, 0.0f), vec2(1.0f, 0.0f), vec3(),

                         vec3(size, 0.0f, size), vec3(0.0f, 0.0f, 1.0f), vec2(1.0f, 1.0f), vec3(),
                         vec3(0.0f, 0.0f, size), vec3(0.0f, 0.0f, 1.0f), vec2(0.0f, 1.0f), vec3(),
                         vec3(0.0f, -size, size), vec3(0.0f, 0.0f, 1.0f), vec2(0.0f, 0.0f), vec3(),
                         vec3(size, -size, size), vec3(0.0f, 0.0f, 1.0f), vec2(1.0f, 0.0f), vec3(),
                         //
                         vec3(0.0f, 0.0f, size), vec3(-1.0f, 0.0f, 0.0f), vec2(1.0f, 1.0f), vec3(),
                         vec3(0.0f, 0.0f, 0.0f), vec3(-1.0f, 0.0f, 0.0f), vec2(0.0f, 1.0f), vec3(),
                         vec3(0.0f, -size, 0.0f), vec3(-1.0f, 0.0f, 0.0f), vec2(0.0f, 0.0f), vec3(),
                         vec3(0.0f, -size, size), vec3(-1.0f, 0.0f, 0.0f), vec2(1.0f, 0.0f), vec3(),

                         vec3(0.0f, 0.0f, size), vec3(0.0f, 1.0f, 0.0f), vec2(1.0f, 1.0f), vec3(),
                         vec3(size, 0.0f, size), vec3(0.0f, 1.0f, 0.0f), vec2(0.0f, 1.0f), vec3(),
                         vec3(size, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f), vec2(0.0f, 0.0f), vec3(),
                         vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f), vec2(1.0f, 0.0f), vec3(),

                         vec3(size, -size, size), vec3(0.0f, -1.0f, 0.0f), vec2(1.0f, 1.0f), vec3(),
                         vec3(0.0f, -size, size), vec3(0.0f, -1.0f, 0.0f), vec2(0.0f, 1.0f), vec3(),
                         vec3(0.0f, -size, 0.0f), vec3(0.0f, -1.0f, 0.0f), vec2(0.0f, 0.0f), vec3(),
                         vec3(size, -size, 0.0f), vec3(0.0f, -1.0f, 0.0f), vec2(1.0f, 0.0f),
                         vec3()};

   for (unsigned int i = 0; i < 24; i++)
   {
      cube.vertices.push_back(verticies[i]);
   }


   uint8_t indieces[] = {0,  1,  2,  0,  2,  3,  4,  5,  6,  4,  6,  7,  8,  9,  10, 8,  10, 11,
                          12, 13, 14, 12, 14, 15, 16, 17, 18, 16, 18, 19, 20, 21, 22, 20, 22, 23};
   for (unsigned int i = 0; i < 36; i++)
   {
      cube.indices.push_back(indieces[i]);
   }
  /* Texture tmp[2];
   tmp[0].LoadTexture("./Models/textures/177.png", "diffuse_map");
   tmp[1].LoadTexture("./Models/textures/177_norm.png", "depth_map");
   cube.textures.push_back(tmp[0]);
   cube.textures.push_back(tmp[1]);*/
   cube.SetupMesh();
   meshes.push_back(cube);
}
