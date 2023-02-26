#pragma once

#include "app/window.hpp"
#include "input/input_listener.hpp"
#include "scene/scene.hpp"

#include <memory>

namespace shady::app {

class Shady : public input::InputListener
{
 public:
   Shady() = default;
   Shady(const Shady&) = delete;
   Shady(Shady&&) = delete;
   Shady& operator=(const Shady&) = delete;
   Shady& operator=(Shady&&) = delete;
  ~Shady() override = default;

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
  // Application settings
   Window m_window;
   int32_t m_windowWidth = 1920;
   int32_t m_windowHeight = 1080;

   scene::Scene m_currentScene;

   bool m_active = true;
};

} // namespace shady::app
