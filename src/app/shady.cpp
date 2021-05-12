#include "app/shady.hpp"
#include "app/input/input_manager.hpp"
#include "trace/logger.hpp"
#include "utils/file_manager.hpp"
#include "gui/gui.hpp"
#include "vulkan/vulkan_common.hpp"
#include "scene/light.hpp"
#include "scene/perspective_camera.hpp"

#include "render/vulkan/vulkan_renderer.hpp"
#include <GLFW/glfw3.h>

namespace shady::app {

using namespace shady::render::vulkan;
scene::Model model;

void
Shady::Init()
{
   m_window = std::make_unique< Window >(m_windowWidth, m_windowHeight, "Shady");

   input::InputManager::Init(m_window->GetWindowHandle());
   input::InputManager::RegisterForKeyInput(this);
   input::InputManager::RegisterForMouseButtonInput(this);
   input::InputManager::RegisterForMouseMovementInput(this);
   input::InputManager::RegisterForMouseScrollInput(this);

   Data::m_light =
      std::make_unique< scene::Light >(glm::vec3(0.0f, 450.0f, 0.0f), glm::vec3(1.0f, 0.8f, 0.7f),
                                       scene::LightType::DIRECTIONAL_LIGHT);
   Data::m_camera = std::make_unique< scene::PerspectiveCamera >(70.0f, 16.0f / 9.0f, 0.1f, 500.0f);

   render::vulkan::VulkanRenderer::Initialize(m_window->GetWindowHandle());

   model = scene::Model((utils::FileManager::MODELS_DIR / "sponza" / "glTF" / "sponza.gltf").string(),
                        scene::LoadFlags::FlipUV);

   model.ScaleModel(glm::vec3(0.1f, 0.1f, 0.1f));
   model.Draw();

   render::vulkan::VulkanRenderer::CreateRenderPipeline();

   m_currentScene.LoadDefault();
}

void
Shady::MainLoop()
{
   while (m_active)
   {
      m_window->Clear();

      OnUpdate();

      // m_currentScene.Render(m_windowWidth, m_windowHeight);
      // render::vulkan::VulkanRenderer::view_mat = Data::m_camera->GetView();
      // render::vulkan::VulkanRenderer::proj_mat = Data::m_camera->GetProjection();
      render::vulkan::VulkanRenderer::proj_mat = Data::m_camera->GetViewProjection();

      render::vulkan::VulkanRenderer::camera_pos = glm::vec4(Data::m_camera->GetPosition(), 0.0f);
      render::vulkan::VulkanRenderer::light_pos = glm::vec4(Data::m_light->GetPosition(), 0.0f);
     /* render::vulkan::VulkanRenderer::view_mat = m_currentScene.GetCamera().GetView();
      render::vulkan::VulkanRenderer::proj_mat = m_currentScene.GetCamera().GetProjection();
      render::vulkan::VulkanRenderer::camera_pos =
         glm::vec4(m_currentScene.GetCamera().GetPosition(), 0.0f);

      render::vulkan::VulkanRenderer::light_pos =
         glm::vec4(m_currentScene.GetLight().GetPosition(), 0.0f);*/

      // render::vulkan::VulkanRenderer::Draw();
      app::gui::Gui::UpdateUI({m_windowWidth, m_windowHeight});
      render::vulkan::VulkanRenderer::DrawDeferred();

      // m_gui.Render({m_windowWidth, m_windowHeight});

      m_window->SwapBuffers();
   }
}

void
Shady::OnUpdate()
{
   constexpr auto cameraMoveBy = 0.05f;
   constexpr auto lightMoveBy = 0.02f;

   input::InputManager::PollEvents();

   if (input::InputManager::CheckKeyPressed(GLFW_KEY_W))
   {
      m_currentScene.GetCamera().MoveCamera({0.0f, cameraMoveBy});
      Data::m_camera->MoveCamera({0.0f, cameraMoveBy});
   }
   if (input::InputManager::CheckKeyPressed(GLFW_KEY_S))
   {
      m_currentScene.GetCamera().MoveCamera({0.0f, -cameraMoveBy});
      Data::m_camera->MoveCamera({0.0f, -cameraMoveBy});
   }
   if (input::InputManager::CheckKeyPressed(GLFW_KEY_A))
   {
      m_currentScene.GetCamera().MoveCamera({-cameraMoveBy, 0.0f});
      Data::m_camera->MoveCamera({-cameraMoveBy, 0.0f});
   }
   if (input::InputManager::CheckKeyPressed(GLFW_KEY_D))
   {
      m_currentScene.GetCamera().MoveCamera({cameraMoveBy, 0.0f});
      Data::m_camera->MoveCamera({cameraMoveBy, 0.0f});
   }

   if (input::InputManager::CheckKeyPressed(GLFW_KEY_LEFT))
   {
      m_currentScene.GetLight().MoveBy({lightMoveBy, 0.0f, 0.0f});
      Data::m_light->MoveBy({lightMoveBy, 0.0f, 0.0f});
   }
   if (input::InputManager::CheckKeyPressed(GLFW_KEY_UP))
   {
      m_currentScene.GetLight().MoveBy({0.0f, -lightMoveBy, 0.0f});
      Data::m_light->MoveBy({0.0f, lightMoveBy, 0.0f});
   }
   if (input::InputManager::CheckKeyPressed(GLFW_KEY_DOWN))
   {
      m_currentScene.GetLight().MoveBy({0.0f, lightMoveBy, 0.0f});
      Data::m_light->MoveBy({0.0f, -lightMoveBy, 0.0f});
   }
   if (input::InputManager::CheckKeyPressed(GLFW_KEY_RIGHT))
   {
      m_currentScene.GetLight().MoveBy({-lightMoveBy, 0.0f, 0.0f});
      Data::m_light->MoveBy({-lightMoveBy, 0.0f, 0.0f});
   }

   if (input::InputManager::CheckKeyPressed(GLFW_KEY_R))
   {
      Data::m_camera->SetView(Data::m_light->GetViewMat());
   }
   if (input::InputManager::CheckKeyPressed(GLFW_KEY_T))
   {
      Data::m_camera->SetViewProjection(Data::m_light->GetLightSpaceMat());
   }
}

void
Shady::KeyCallback(const input::KeyEvent& event)
{
   switch (event.m_key)
   {
      case GLFW_KEY_ESCAPE: {
         m_active = false;
      }
      break;
   }
}

void
Shady::MouseButtonCallback(const input::MouseButtonEvent& /*event*/)
{
}

void
Shady::CursorPositionCallback(const input::CursorPositionEvent& event)
{
   if (input::InputManager::CheckButtonPressed(GLFW_MOUSE_BUTTON_LEFT))
   {
      m_currentScene.GetCamera().MouseMovement({event.m_xDelta, event.m_yDelta});
      Data::m_camera->MouseMovement({event.m_xDelta, event.m_yDelta});
   }

   // const auto upVec = m_currentScene.GetCamera().GetUpVec();
   // const auto lookAt = m_currentScene.GetCamera().GetLookAtVec();
   // trace::Logger::Debug("LookAtVec=({},{},{}); UpVec=({},{},{})", lookAt.x, lookAt.y, lookAt.z,
   //                      upVec.x, upVec.y, upVec.z);
}

void
Shady::MouseScrollCallback(const input::MouseScrollEvent& /*event*/)
{
}

} // namespace shady::app
