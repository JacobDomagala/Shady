#pragma once

#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <string>

#include "scene/camera.hpp"

namespace shady::scene {

class Skybox
{
 public:
   void
   LoadCubeMap(const std::string& directoryPath);

   void
   Draw(const Camera& camera, uint32_t windowWidth, uint32_t windowHeight);

 private:
   //render::TexturePtr m_cubeTexture;
   //std::shared_ptr<render::Shader> m_shader;
   //std::shared_ptr<render::VertexArray> m_vertexArray;
};

} // namespace shady::scene
