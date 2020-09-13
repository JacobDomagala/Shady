#pragma once

#include "context.hpp"

namespace shady::render::opengl {

class OpenGLContext : public Context
{
 public:
   explicit OpenGLContext(GLFWwindow* window);
   virtual ~OpenGLContext() = default;

   void
   Init() override;

   void
   SwapBuffers() override;

 private:
   GLFWwindow* m_windowHandle = nullptr;
};

} // namespace shady::render::opengl