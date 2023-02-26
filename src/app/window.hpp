#pragma once

#include <glm/glm.hpp>
#include <string>

#include <vulkan/vulkan.h>

struct GLFWwindow;

namespace shady::app {

class Window
{
 public:
   Window() = default;
   Window(const Window&) = delete;
   Window(Window&&) = delete;
   Window& operator=(const Window&) = delete;
   Window& operator=(Window&&) = delete;

   Window(int32_t width, int32_t height, const std::string& title);
   ~Window();

   void
   Create(int32_t width, int32_t height, const std::string& title);

   void
   ShutDown();

   [[nodiscard]]
   glm::ivec2
   GetSize() const;

   void
   Resize(int32_t newWidth, int32_t newHeight);

   void
   SetIcon(const std::string& file);

   void
   Clear();

   void
   SwapBuffers();

   // true -> mouse visible and not wrapped
   // false -> mouse is disabled (hidden and wrapped)
   void
   ShowCursor(bool choice);

   // update and get cursor position <-1, 1>
   // with positive 'y' is up
   [[nodiscard]]
   glm::vec2
   GetCursorScreenPosition(const glm::mat4& projectionMatrix);

   // update and get cursor position <-1, 1>
   // with positive 'y' is down
   [[nodiscard]]
   glm::vec2
   GetCursorNormalized();

   // return current cursor position on window
   [[nodiscard]]
   glm::vec2
   GetCursor();

   [[nodiscard]]
   GLFWwindow*
   GetWindowHandle();

 private:
   bool m_created = false;
   int32_t m_width = 0;
   int32_t m_height = 0;
   std::string m_title = "Default Title";

   GLFWwindow* m_pWindow = nullptr;
};

} // namespace shady::app
