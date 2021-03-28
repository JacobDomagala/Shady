#pragma once

#include <memory>
#include <unordered_map>
#include <vector>

#include <glm/glm.hpp>

struct GLFWwindow;

namespace shady::app::gui {

class Gui
{
 public:
   void
   Init(GLFWwindow* windowHandle);

   void
   Shutdown();

   void
   Render(const glm::ivec2& windowSize, uint32_t shadowMapID);
};

} // namespace shady::app::gui
