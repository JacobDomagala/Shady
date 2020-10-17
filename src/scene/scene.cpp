#pragma once

#include "scene/scene.hpp"
#include "render/renderer_3D.hpp"
#include "scene/perspective_camera.hpp"
#include "utils/file_manager.hpp"

namespace shady::scene {

void
Scene::AddCamera()
{
}

void
Scene::AddModel(const std::string& fileName)
{
   auto model = std::make_unique< Model >(fileName);
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
  //  m_light->BeginRenderToLightmap();
  //  render::Renderer3D::BeginScene(*m_camera);

  //  for (auto& model : m_models)
  //  {
  //     model->Draw();
  //  }

  //  render::Renderer3D::EndScene();
  //  m_light->EndRenderToLightmap();

   // Second pass -> use lightmap generated to render shadows
   m_skybox.Draw(*m_camera);
  //  render::Renderer3D::BeginScene(*m_camera);
  //  for (auto& model : m_models)
  //  {
  //     model->Draw();
  //  }
  //  render::Renderer3D::EndScene();
}

void
Scene::LoadDefault()
{
   m_camera = std::make_unique< PerspectiveCamera >(90.0f, 16.0f / 9.0f, 0.1f, 10.0f);
   m_skybox.LoadCubeMap((utils::FileManager::TEXTURES_DIR / "cloudy" / "bluecloud").u8string());
}

} // namespace shady::scene