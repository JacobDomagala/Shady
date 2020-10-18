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
   m_window = std::make_unique< Window >(1920, 1080, "Shady");
   render::Renderer::Init();

   input::InputManager::Init(m_window->GetWindowHandle());
   input::InputManager::RegisterForKeyInput(this);
   input::InputManager::RegisterForMouseButtonInput(this);
   input::InputManager::RegisterForMouseMovementInput(this);
   input::InputManager::RegisterForMouseScrollInput(this);

   m_currentScene.LoadDefault();
}

void
Shady::MainLoop()
{
   render::RenderCommand::SetClearColor({0.4f, 0.1f, 0.3f, 1.0f});

   while (m_active)
   {
      m_window->Clear();

      OnUpdate();

      m_currentScene.Render();

      m_window->SwapBuffers();
   }
}

void
Shady::OnUpdate()
{
   input::InputManager::PollEvents();
}

void
Shady::KeyCallback(const input::KeyEvent& event)
{
   auto& mainCamera = m_currentScene.GetCamera();
   switch (event.m_key)
   {
      case GLFW_KEY_ESCAPE: {
         m_active = false;
      }
      break;
      case GLFW_KEY_W: {
         //  mainCamera.MoveBy({0.0f, 0.0f, 0.01f});
      }
      break;

      case GLFW_KEY_S: {
         // mainCamera.MoveBy({0.0f, 0.0f, -0.01f});
      }
      break;

      case GLFW_KEY_A: {
         // mainCamera.MoveBy({0.01f, 0.0f, 0.0f});
      }
      break;

      case GLFW_KEY_D: {
         // mainCamera.MoveBy({-0.01f, 0.0f, 0.0f});
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
