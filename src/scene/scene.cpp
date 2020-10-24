#pragma once

#include "scene/scene.hpp"
#include "render/render_command.hpp"
#include "render/renderer_3D.hpp"
#include "scene/perspective_camera.hpp"
#include "time/scoped_timer.hpp"
#include "utils/file_manager.hpp"


#include <fmt/format.h>

namespace shady::scene {

void
Scene::AddCamera(CameraType type, const glm::vec3& position,
                 std::initializer_list< float > constructParams)
{
}

scene::Camera&
Scene::GetCamera()
{
   return *m_camera;
}

void
Scene::AddModel(const std::string& fileName)
{
   auto model = std::make_unique< Model >(fileName);
   model->ScaleModel(glm::vec3{10.0f, 10.0f, 10.0f});
   m_models.push_back(std::move(model));
}

void
Scene::AddLight(LightType type, const glm::vec3& position, const glm::vec3& color)
{
   m_light = std::make_unique< Light >(position, color, type);
}

void
Scene::Render()
{
   // For now we only use single light
   // First pass -> render depth from camera POV to texture
   // m_light->BeginRenderToLightmap();
   // render::Renderer3D::BeginScene(*m_camera);

   // for (auto& model : m_models)
   //{
   //   model->Draw();
   //}

   // render::Renderer3D::EndScene();
   // m_light->EndRenderToLightmap();

   // Second pass -> use lightmap generated to render shadows
   m_skybox.Draw(*m_camera);
   render::Renderer3D::BeginScene(*m_camera);

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
   m_camera = std::make_unique< PerspectiveCamera >(70.0f, 16.0f / 9.0f, 0.1f, 500.0f);
   m_skybox.LoadCubeMap((utils::FileManager::TEXTURES_DIR / "skybox" / "default").u8string());
   AddModel((utils::FileManager::MODELS_DIR / "Crate" / "Crate1.obj").u8string());
}

} // namespace shady::scene