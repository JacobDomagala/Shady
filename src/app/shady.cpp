#include "app/shady.hpp"
#include "app/input/input_manager.hpp"
#include "render/render_command.hpp"
#include "render/renderer.hpp"
#include "utils/file_manager.hpp"

namespace shady::app {

void
Shady::Init()
{
   m_window = std::make_unique< Window >(1280, 760, "Shady");
   render::Renderer::Init();
   input::InputManager::Init(m_window->GetWindowHandle());
   m_currentScene.LoadDefault();
}

void
Shady::MainLoop()
{
   render::RenderCommand::SetClearColor({0.4f, 0.1f, 0.3f, 1.0f});

   while (1)
   {
      shady::app::input::InputManager::PollEvents();
      m_window->Clear();

      m_currentScene.Render();

      m_window->SwapBuffers();
   }
}

} // namespace shady::app
