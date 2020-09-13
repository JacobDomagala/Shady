#ifndef MODEL_H
#define MODEL_H

#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb_image.h>


#include "../DISPLAY/Camera.h"
#include "../Light.h"
#include "Mesh.h"
#include "app/window.hpp"

using glm::mat4;
using glm::vec3;
using std::string;
using std::vector;

struct Model
{
   // Model View Projection matrices
   mat4 modelMatrix;
   mat4 viewMatrix;
   mat4 projectionMatrix;

   // Model matrix data
   vec3 translateValue;
   vec3 scaleValue;
   vec3 rotateValue;
   float rotateAngle;

   Model();
   void
   LoadModelFromFile(char* path);
   void
   ScaleModel(vec3 scale);
   void
   TranslateModel(vec3 translate);
   void
   RotateModel(vec3 rotate, float angle);

   void
   Draw(shady::app::Window* window, Camera camera, Light* lights, Shader shader);

   uint32_t modelMatrixUniformLocation;

   uint32_t programID;
   vector< Mesh > meshes;
   string directory;
   //vector< Texture > textures_loaded;

   void
   LoadModel(string path);

   void
   ProcessNode(aiNode* node, const aiScene* scene);
   Mesh
   ProcessMesh(aiMesh* mesh, const aiScene* scene);

   //vector< Texture >
   //LoadMaterialTextures(aiMaterial* mat, aiTextureType type, char* typeName);

   void
   CreateCube(int size);
};


#endif