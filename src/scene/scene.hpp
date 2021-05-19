#pragma once

#include "scene/camera.hpp"
#include "scene/light.hpp"
#include "scene/model.hpp"
#include "scene/skybox.hpp"

#include <memory>
#include <vector>

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
   Render(uint32_t windowWidth, uint32_t windowHeight);

   void
   LoadDefault();

 private:
   // Skybox m_skybox;
   std::unique_ptr< Camera > m_camera;
   std::vector< std::unique_ptr< Model > > m_models;
   std::unique_ptr< Light > m_light;
};

} // namespace shady::scene
