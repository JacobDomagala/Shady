#pragma once

#include "app/window.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <vector>

namespace shady::scene {

class Skybox
{
 public:
   void
   LoadCubeMap(std::string folderPath);
   void
   Draw(shady::app::Window* window /*,Camera camera, Shader shader*/);

 private:
   std::vector< std::string > faces;
   uint32_t textureID;
   // Shader skyBoxShaders;
   uint32_t skyboxVAO, skyboxVBO;
};

} // namespace shady::scene