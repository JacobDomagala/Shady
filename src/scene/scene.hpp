#pragma once

#include "scene/light.hpp"
#include "scene/skybox.hpp"
#include "scene/model.hpp"

#include <memory>
#include <vector>

namespace shady::render {
class Camera;
}

namespace shady::scene {

class Scene
{
 public:
   void
   AddCamera();

   void
   AddModel(const std::string& fileName);

   void
   AddLight(LightType type, const glm::vec3& position, const glm::vec3& color);

   void
   Render();

   void
   LoadDefault();

 private:
   Skybox m_skybox;
   std::unique_ptr< scene::Camera > m_camera;
   std::vector< std::unique_ptr< Model > > m_models;
   std::unique_ptr< Light > m_light;
   std::shared_ptr< render::Shader > m_mainShader;
};

} // namespace shady::scene