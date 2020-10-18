#pragma once

#include "app/window.hpp"
#include "input/input_listener.hpp"
#include "scene/scene.hpp"


#include <memory>

namespace shady::app {

class Shady : public input::InputListener
{
 public:
   void
   Init();

   void
   MainLoop();

   // InputListener overrides
 public:
   void
   KeyCallback(const input::KeyEvent& event) override;

   void
   MouseButtonCallback(const input::MouseButtonEvent& event) override;

   void
   CursorPositionCallback(const input::CursorPositionEvent& event) override;

   void
   MouseScrollCallback(const input::MouseScrollEvent& event) override;

 private:
   void
   OnUpdate();

 private:
   std::unique_ptr< Window > m_window;
   scene::Scene m_currentScene;
   bool m_active = true;
};

} // namespace shady::app
