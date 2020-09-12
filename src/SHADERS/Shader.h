#ifndef SHADER_H
#define SHADER_H

#include <glm/glm.hpp>
#include <iostream>
#include <string>

#include "render/renderer.hpp"

struct Shader
{
   GLuint programID;

   void
   LoadShaders(char* vertexFile, char* fragmentFile);

   void
   UseProgram();

   bool CheckStatus(GLuint, PFNGLGETSHADERIVPROC, PFNGLGETSHADERINFOLOGPROC, GLenum);
   bool CheckShaderStatus(GLuint);
   bool CheckProgramStatus(GLuint);

   char*
   ReadFile(char* fileName);
   void
   AddTess();
};

#endif