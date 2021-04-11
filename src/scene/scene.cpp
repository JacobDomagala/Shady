#include "scene/scene.hpp"
#include "render/render_command.hpp"
#include "render/renderer_3D.hpp"
#include "scene/perspective_camera.hpp"
#include "time/scoped_timer.hpp"
#include "utils/file_manager.hpp"

#include <fmt/format.h>
#include <glad/glad.h>


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
Scene::AddLight(LightType type, const glm::vec3& position, const glm::vec3& color)
{
   // m_light = std::make_unique< Light >(position, color, type);
}

Light&
Scene::GetLight()
{
   return *m_light;
}

void
Scene::Render(uint32_t windowWidth, uint32_t windowHeight)
{
   // SCOPED_TIMER("Scene::Render");
   // m_lightSphere->TranslateModel(m_light->GetPosition());

   ////////////////////////////////////////////////////////
   ///////////////////// FIRST PASS ///////////////////////
   ////////////////////////////////////////////////////////

   // For now we only use single light
   // First pass -> render depth from camera POV to texture
   render::RenderCommand::Clear();
   render::Renderer3D::BeginScene({windowWidth, windowHeight}, *m_camera, *m_light, true);

   for (auto& model : m_models)
   {
      model->Draw();
   }

   render::Renderer3D::EndScene();

   ////////////////////////////////////////////////////////
   ///////////////////// SECOND PASS //////////////////////
   ////////////////////////////////////////////////////////

   // Second pass -> use lightmap generated to render shadows
   render::RenderCommand::Clear();

  // m_skybox.Draw(*m_camera, windowWidth, windowHeight);

   render::Renderer3D::BeginScene({windowWidth, windowHeight}, *m_camera, *m_light);

   for (auto& model : m_models)
   {
      model->Draw();
   }

   render::Renderer3D::EndScene();
}

void
Scene::LoadDefault()
{
   time::ScopedTimer loadScope("Scene::LoadDefault");

    m_light = std::make_unique< Light >(glm::vec3{0.0f, 200.0f, 0.0f}, glm::vec3{1.0f, 0.7f, 0.8f},
                                        LightType::DIRECTIONAL_LIGHT);

   m_camera = std::make_unique< PerspectiveCamera >(70.0f, 16.0f / 9.0f, 0.1f, 500.0f);
   // m_skybox.LoadCubeMap((utils::FileManager::TEXTURES_DIR / "skybox" / "default").u8string());

   // AddModel((utils::FileManager::MODELS_DIR / "sponza" / "sponza.obj").u8string(),
   //          LoadFlags::FlipUV);

   // m_models.back()->ScaleModel({0.1f, 0.1f, 0.1f});
}

} // namespace shady::scene
