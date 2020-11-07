#pragma once

#include "scene/light.hpp"
#include "scene/model.hpp"
#include "scene/skybox.hpp"


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
   AddCamera(CameraType type, const glm::vec3& position,
             std::initializer_list< float > constructParams);

   scene::Camera&
   GetCamera();

   void
   AddModel(const std::string& fileName, LoadFlags additionalFlags = LoadFlags::None);

   void
   AddLight(LightType type, const glm::vec3& position, const glm::vec3& color);

   Light&
   GetLight();

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

   Model* m_lightSphere = nullptr;
};

} // namespace shady::scene