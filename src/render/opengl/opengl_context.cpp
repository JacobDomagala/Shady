#include "opengl_context.hpp"
#include "trace/logger.hpp"
#include "utils/assert.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace shady::render::opengl {

OpenGLContext::OpenGLContext(GLFWwindow* window) : m_windowHandle(window)
{
}

void
OpenGLContext::Init()
{
   glfwMakeContextCurrent(m_windowHandle);

   utils::Assert(gladLoadGLLoader((GLADloadproc)glfwGetProcAddress), "Failed to initialize Glad!");

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
