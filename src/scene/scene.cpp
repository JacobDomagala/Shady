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
Scene::AddModel(const std::string& fileName, LoadFlags additionalFlags)
{
   auto model = std::make_unique< Model >(fileName, additionalFlags);
   m_models.push_back(std::move(model));
}

void
Scene::AddLight(LightType type, const glm::vec3& position, const glm::vec3& color)
{
   m_light = std::make_unique< Light >(position, color, type);
}

Light&
Scene::GetLight()
{
   return *m_light;
}

void
Scene::Render()
{
   SCOPED_TIMER("Scene::Render");


   m_lightSphere->TranslateModel(m_light->GetPosition());


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
   {
      SCOPED_TIMER("SKYBOX");
      m_skybox.Draw(*m_camera);
   }

   {
      SCOPED_TIMER("MODELS");
      render::Renderer3D::BeginScene(*m_camera, *m_light);

      for (auto& model : m_models)
      {
         model->Draw();
      }

      render::Renderer3D::EndScene();
   }
}

void
Scene::LoadDefault()
{
   m_light = std::make_unique< Light >(glm::vec3{2.0f, 5.0f, -10.0f}, glm::vec3{1.0f, 0.7f, 0.8f},
                                       LightType::DIRECTIONAL_LIGHT);
   time::ScopedTimer loadScope("Scene::LoadDefault");
   m_camera = std::make_unique< PerspectiveCamera >(70.0f, 16.0f / 9.0f, 0.1f, 10000.0f);
   m_skybox.LoadCubeMap((utils::FileManager::TEXTURES_DIR / "skybox" / "default").u8string());

  AddModel((utils::FileManager::MODELS_DIR / "sponza" / "sponza.obj").u8string(), LoadFlags::FlipUV);
//   AddModel((utils::FileManager::MODELS_DIR / "Crate" / "Crate1.obj").u8string());
//    auto& crate = m_models.back();
//    crate->TranslateModel(glm::vec3{-2.0f, 0.0f, -3.0f});
//    crate->RotateModel(glm::vec3{0.0f, 1.0f, 0.0f}, 20.0f);
//    crate->GetMeshes().front().AddTexture(
//       render::TextureLibrary::GetTexture(render::TextureType::NORMAL_MAP, "196_norm.png"));
//    crate->GetMeshes().front().AddTexture(
//       render::TextureLibrary::GetTexture(render::TextureType::SPECULAR_MAP, "196_s.png"));

   // AddModel((utils::FileManager::MODELS_DIR / "suzanne.obj").u8string());
   // auto& suzanne = m_models.back();
   // suzanne->GetMeshes().front().AddTexture(render::TextureLibrary::GetTexture(
   //    render::TextureType::DIFFUSE_MAP, "metal_hammered_diffuse.jpg"));
   // suzanne->GetMeshes().front().AddTexture(render::TextureLibrary::GetTexture(
   //    render::TextureType::NORMAL_MAP, "metal_hammered_norm.jpg"));
   // suzanne->GetMeshes().front().AddTexture(render::TextureLibrary::GetTexture(
   //    render::TextureType::SPECULAR_MAP, "metal_hammered_rough.jpg"));
   // suzanne->ScaleModel({0.5f, 0.5f, 0.5f});
   // suzanne->TranslateModel(glm::vec3{2.0f, 0.0f, -2.0f});

   // AddModel((utils::FileManager::MODELS_DIR / "floor" / "floor.obj").u8string());
   // auto& floor = m_models.back();
   // floor->TranslateModel({0.0f, -1.5f, 0.0f});
   // floor->GetMeshes().front().AddTexture(render::TextureLibrary::GetTexture(
   //    render::TextureType::DIFFUSE_MAP, "metal_hammered_diffuse.jpg"));

   AddModel((utils::FileManager::MODELS_DIR / "sphere" / "sphere.obj").u8string());
   m_lightSphere = m_models.back().get();

   // for(auto i : {0,1,2,3,4,5, 6,7,8,9,10,11,12,13,14,15})
   //    AddModel((utils::FileManager::MODELS_DIR / "sphere" / "sphere.obj").u8string());

}

} // namespace shady::scene