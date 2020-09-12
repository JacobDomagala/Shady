#ifndef SKYBOX_H
#define SKYBOX_H

#include "DISPLAY/Camera.h"
#include "SHADERS/Shader.h"
#include "app/window.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <vector>

struct SkyBox
{
   std::vector< std::string > faces;
   GLuint textureID;
   Shader skyBoxShaders;
   GLuint skyboxVAO, skyboxVBO;

   void
   LoadCubeMap(std::string folderPath);
   void
   Draw(shady::app::Window* window, Camera camera, Shader shader);
};

#endif