#include "opengl_context.hpp"
#include "trace/logger.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace shady::render::opengl {

OpenGLContext::OpenGLContext(GLFWwindow* window) : m_windowHandle(window)
{
   Init();
}

void
OpenGLContext::Init()
{
   glfwMakeContextCurrent(m_windowHandle);
   int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

   if (!status)
   {
      trace::Logger::Fatal("Failed to initialize Glad!");
   }

   trace::Logger::Info("OpenGL context created!\n\tVendor: {}\n\tRenderer: {}\n\tVersion: {}",
                       glGetString(GL_VENDOR), glGetString(GL_RENDERER), glGetString(GL_VERSION));
}

void
OpenGLContext::SwapBuffers()
{
   glfwMakeContextCurrent(m_windowHandle);
   glfwSwapBuffers(m_windowHandle);
}

} // namespace shady::render::opengl