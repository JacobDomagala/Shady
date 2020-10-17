#pragma once

#include "app/window.hpp"
#include "scene/scene.hpp"

#include <memory>

namespace shady::app {

class Shady
{
 public:
   void
   Init();

   void
   MainLoop();

 private:
   std::unique_ptr< Window > m_window;
   scene::Scene m_currentScene;
};

} // namespace shady::app
