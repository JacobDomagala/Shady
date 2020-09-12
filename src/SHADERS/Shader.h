#ifndef SHADER_H
#define SHADER_H

#include <glm/glm.hpp>
#include <iostream>
#include <string>

#include "render/renderer.hpp"

struct Shader
{
   uint32_t programID;

   void
   LoadShaders(char* vertexFile, char* fragmentFile);

   void
   UseProgram();

   //bool CheckStatus(GLuint, PFNGLGETSHADERIVPROC, PFNGLGETSHADERINFOLOGPROC, GLenum);
   //bool CheckShaderStatus(uint32_t);
   //bool CheckProgramStatus(uint32_t);

   char*
   ReadFile(char* fileName);
   void
   AddTess();
};

#endif