#include "app/shady.hpp"
#include "app/input/input_manager.hpp"
#include "render/render_command.hpp"
#include "render/renderer.hpp"
#include "trace/logger.hpp"
#include "utils/file_manager.hpp"


#include <GLFW/glfw3.h>

namespace shady::app {

void
Shady::Init()
{
   m_window = std::make_unique< Window >(m_windowWidth, m_windowHeight, "Shady");
   render::Renderer::Init();

   input::InputManager::Init(m_window->GetWindowHandle());
   input::InputManager::RegisterForKeyInput(this);
   input::InputManager::RegisterForMouseButtonInput(this);
   input::InputManager::RegisterForMouseMovementInput(this);
   input::InputManager::RegisterForMouseScrollInput(this);

   m_currentScene.LoadDefault();

   render::RenderCommand::SetClearColor({0.4f, 0.1f, 0.3f, 1.0f});

   m_gui.Init(m_window->GetWindowHandle());
}

void
Shady::MainLoop()
{
   while (m_active)
   {
      m_window->Clear();

      OnUpdate();

      m_currentScene.Render(m_windowWidth, m_windowHeight);
      m_gui.Render({m_windowWidth, m_windowHeight}, m_currentScene.GetLight().GetDepthMapID());

      m_window->SwapBuffers();
   }
}

void
Shady::OnUpdate()
{
   input::InputManager::PollEvents();

   if (input::InputManager::CheckKeyPressed(GLFW_KEY_W))
   {
      m_currentScene.GetCamera().MoveCamera({0.0f, 2.0f});
   }
   if (input::InputManager::CheckKeyPressed(GLFW_KEY_S))
   {
      m_currentScene.GetCamera().MoveCamera({0.0f, -2.0f});
   }
   if (input::InputManager::CheckKeyPressed(GLFW_KEY_A))
   {
      m_currentScene.GetCamera().MoveCamera({-2.0f, 0.0f});
   }
   if (input::InputManager::CheckKeyPressed(GLFW_KEY_D))
   {
      m_currentScene.GetCamera().MoveCamera({2.0f, 0.0f});
   }

   if (input::InputManager::CheckKeyPressed(GLFW_KEY_LEFT))
   {
      m_currentScene.GetLight().MoveBy({0.5f, 0.0f, 0.0f});
   }
   if (input::InputManager::CheckKeyPressed(GLFW_KEY_UP))
   {
      m_currentScene.GetLight().MoveBy({0.0f, 0.5f, 0.0f});
   }
   if (input::InputManager::CheckKeyPressed(GLFW_KEY_DOWN))
   {
      m_currentScene.GetLight().MoveBy({0.0f, -0.5f, 0.0f});
   }
   if (input::InputManager::CheckKeyPressed(GLFW_KEY_RIGHT))
   {
      m_currentScene.GetLight().MoveBy({-0.5f, 0.0f, 0.0f});
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
Shady::MouseButtonCallback(const input::MouseButtonEvent& event)
{
}

void
Shady::CursorPositionCallback(const input::CursorPositionEvent& event)
{
   m_currentScene.GetCamera().MouseMovement({event.m_xDelta, event.m_yDelta});
   // const auto upVec = m_currentScene.GetCamera().GetUpVec();
   // const auto lookAt = m_currentScene.GetCamera().GetLookAtVec();
   // trace::Logger::Debug("LookAtVec=({},{},{}); UpVec=({},{},{})", lookAt.x, lookAt.y, lookAt.z,
   //                      upVec.x, upVec.y, upVec.z);
}

void
Shady::MouseScrollCallback(const input::MouseScrollEvent& event)
{
}

} // namespace shady::app
