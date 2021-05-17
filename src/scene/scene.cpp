#include "scene/scene.hpp"
#include "scene/perspective_camera.hpp"
#include "time/scoped_timer.hpp"
#include "utils/file_manager.hpp"

#include <fmt/format.h>


namespace shady::scene {


void
Scene::AddCamera(CameraType /*type*/, const glm::vec3& /*position*/,
                 std::initializer_list< float > /*constructParams*/)
{
}

scene::Camera&
Scene::GetCamera()
{
   return *m_camera;
}

void
Scene::AddModel(const std::string& fileName, LoadFlags additionalFlags)
{
   auto model = std::make_unique< Model >(fileName, additionalFlags);
   m_models.push_back(std::move(model));
}

void
Scene::AddLight(LightType /*type*/, const glm::vec3& /*position*/, const glm::vec3& /*color*/)
{

}

Light&
Scene::GetLight()
{
   return *m_light;
}

void
Scene::Render(uint32_t /*windowWidth*/, uint32_t /*windowHeight*/)
{

}

void
Scene::LoadDefault()
{
   time::ScopedTimer loadScope("Scene::LoadDefault");

   m_light = std::make_unique< Light >(glm::vec3{0.0f, -200.0f, 0.0f}, glm::vec3{1.0f, 0.7f, 0.8f},
                                        LightType::DIRECTIONAL_LIGHT);

   m_camera = std::make_unique< PerspectiveCamera >(70.0f, 16.0f / 9.0f, 0.1f, 500.0f,
                                                    glm::vec3(0.0f, 20.0f, 0.0f));
}

} // namespace shady::scene
