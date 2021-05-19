#include "app/shady.hpp"
#include "app/input/input_manager.hpp"
#include "common.hpp"
#include "gui/gui.hpp"
#include "scene/light.hpp"
#include "scene/perspective_camera.hpp"
#include "trace/logger.hpp"
#include "utils/file_manager.hpp"

#include "render/renderer.hpp"
#include <GLFW/glfw3.h>

namespace shady::app {

using namespace shady::render;
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

   render::Renderer::Initialize(m_window->GetWindowHandle());

   m_currentScene.LoadDefault();

   render::Renderer::CreateRenderPipeline();
}

void
Shady::MainLoop()
{
   while (m_active)
   {
      m_window->Clear();

      input::InputManager::PollEvents();

      // Check if GUI captures the input. This could be moved into some kind
      // of input layer stack, but since we only have UI and application,
      // there's no need for that
      if (!app::gui::Gui::UpdateUI({m_windowWidth, m_windowHeight}, m_currentScene))
      {
         OnUpdate();
      }

      m_currentScene.Render(m_windowWidth, m_windowHeight);

      m_window->SwapBuffers();
   }
}

void
Shady::OnUpdate()
{
   auto& camera = m_currentScene.GetCamera();
   auto& light = m_currentScene.GetLight();

   constexpr auto cameraMoveBy = 0.05f;
   constexpr auto lightMoveBy = 0.02f;

   /*
    *  Camera movement
    */
   if (input::InputManager::CheckKeyPressed(GLFW_KEY_W))
   {
      camera.MoveCamera({0.0f, cameraMoveBy});
   }
   if (input::InputManager::CheckKeyPressed(GLFW_KEY_S))
   {
      camera.MoveCamera({0.0f, -cameraMoveBy});
   }
   if (input::InputManager::CheckKeyPressed(GLFW_KEY_A))
   {
      camera.MoveCamera({-cameraMoveBy, 0.0f});
   }
   if (input::InputManager::CheckKeyPressed(GLFW_KEY_D))
   {
      camera.MoveCamera({cameraMoveBy, 0.0f});
   }

   /*
    * Light movement
    */
   if (input::InputManager::CheckKeyPressed(GLFW_KEY_LEFT))
   {
      light.MoveBy({lightMoveBy, 0.0f, 0.0f});
   }
   if (input::InputManager::CheckKeyPressed(GLFW_KEY_UP))
   {
      light.MoveBy({0.0f, -lightMoveBy, 0.0f});
   }
   if (input::InputManager::CheckKeyPressed(GLFW_KEY_DOWN))
   {
      light.MoveBy({0.0f, lightMoveBy, 0.0f});
   }
   if (input::InputManager::CheckKeyPressed(GLFW_KEY_RIGHT))
   {
      light.MoveBy({-lightMoveBy, 0.0f, 0.0f});
   }

   if (input::InputManager::CheckKeyPressed(GLFW_KEY_R))
   {
      camera.SetView(light.GetViewMat());
   }
   if (input::InputManager::CheckKeyPressed(GLFW_KEY_T))
   {
      camera.SetViewProjection(light.GetLightSpaceMat());
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
   if (input::InputManager::CheckButtonPressed(GLFW_MOUSE_BUTTON_LEFT)
       and !app::gui::Gui::UpdateUI({m_windowWidth, m_windowHeight}, m_currentScene))
   {
      m_currentScene.GetCamera().MouseMovement({event.m_xDelta, event.m_yDelta});
   }
}

void
Shady::MouseScrollCallback(const input::MouseScrollEvent& /*event*/)
{
}

} // namespace shady::app
