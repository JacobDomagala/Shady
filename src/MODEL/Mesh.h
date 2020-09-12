#ifndef MESH_H
#define MESH_H


#include <sstream>
#include <vector>

#include "../SHADERS/Shader.h"
#include "Texture.h"
#include "render/renderer.hpp"
#include <glm/gtc/matrix_transform.hpp>


using glm::vec2;
using glm::vec3;
using std::string;
using std::vector;

struct Vertex
{
   vec3 position;
   vec3 normal;
   vec2 texCoords;
   vec3 tangent;
};

struct Mesh
{
   vector< Vertex > vertices;
   vector< GLuint > indices;
   vector< Texture > textures;
   GLuint programID;


   Mesh(vector< Vertex >* vertices, vector< GLuint >* indices, vector< Texture >* textures);
   void
   AddTexture(char* filePath, textureType textureType);

   void
   Draw(GLuint programID);
   void
   Delete();

   GLuint VAO, VBO, EBO;

   void
   SetupMesh();
};


#endif