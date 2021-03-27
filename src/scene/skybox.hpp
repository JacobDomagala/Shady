#pragma once

#include <glm/gtc/type_ptr.hpp>
#include <vector>

#include "render/texture.hpp"
#include "render/shader.hpp"
#include "render/vertex_array.hpp"
#include "scene/camera.hpp"

namespace shady::scene {

class Skybox
{
 public:
   void
   LoadCubeMap(const std::string& folderPath);

   void
   Draw(const Camera& camera, uint32_t windowWidth, uint32_t windowHeight);

 private:
   render::TexturePtr m_cubeTexture;
   std::shared_ptr<render::Shader> m_shader;
   std::shared_ptr<render::VertexArray> m_vertexArray;
};

} // namespace shady::scene
