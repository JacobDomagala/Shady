#include "scene/scene.hpp"
#include "render/renderer.hpp"
#include "scene/perspective_camera.hpp"
#include "time/scoped_timer.hpp"
#include "utils/file_manager.hpp"

#include <fmt/format.h>


namespace shady::scene {

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

Light&
Scene::GetLight()
{
   return *m_light;
}

void Scene::Render(int32_t /*windowWidth*/, int32_t /*windowHeight*/)
{
   render::Renderer::UpdateUniformBuffer(m_camera.get(), m_light.get());
   render::Renderer::Draw();
}

void
Scene::LoadDefault()
{
   const time::ScopedTimer loadScope("Scene::LoadDefault");

   AddModel((utils::FileManager::MODELS_DIR / "sponza" / "glTF" / "Sponza.gltf").string(),
            scene::LoadFlags::FlipUV);

   m_models.back()->ScaleModel(glm::vec3(0.1f, 0.1f, 0.1f));
   m_models.back()->Submit();

   m_light =
      std::make_unique< scene::Light >(glm::vec3(0.0f, 450.0f, 0.0f), glm::vec3(1.0f, 0.8f, 0.7f),
                                       scene::LightType::DIRECTIONAL_LIGHT);

   m_camera = std::make_unique< scene::PerspectiveCamera >(70.0f, 16.0f / 9.0f, 0.1f, 500.0f,
                                                           glm::vec3(0.0f, 20.0f, 0.0f));
}

} // namespace shady::scene
