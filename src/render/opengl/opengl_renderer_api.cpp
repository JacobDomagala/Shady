
#include "opengl_renderer_api.hpp"
#include "trace/logger.hpp"

#include <glad/glad.h>

namespace shady::render::opengl {

void
OpenGLRendererAPI::Init()
{
   glEnable(GL_DEBUG_OUTPUT);
   glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

   glDebugMessageCallback(
      [](GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
         const GLchar* message, const void* logger) {
         switch (severity)
         {
            case GL_DEBUG_SEVERITY_HIGH:
               trace::Logger::Fatal(std::string(message));
               return;
            case GL_DEBUG_SEVERITY_MEDIUM:
               trace::Logger::Warn(std::string(message));
               return;
            case GL_DEBUG_SEVERITY_LOW:
               trace::Logger::Info(std::string(message));
               return;
            case GL_DEBUG_SEVERITY_NOTIFICATION:
               trace::Logger::Debug(std::string(message));
               return;
         }
      },
      nullptr);

   glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL,
                         GL_FALSE);

   glEnable(GL_MULTISAMPLE);
   glEnable(GL_DEPTH_TEST);
   glDepthMask(GL_TRUE);

   glEnable(GL_CULL_FACE);
   glCullFace(GL_BACK);
   glFrontFace(GL_CCW);

   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void
OpenGLRendererAPI::SetDepthFunc(DepthFunc depthFunc)
{
   glDepthFunc(depthFunc == DepthFunc::LEQUAL ? GL_LEQUAL : GL_LESS);
}

void
OpenGLRendererAPI::EnableDepthTesting()
{
   glEnable(GL_DEPTH_TEST);
}

void
OpenGLRendererAPI::DisableDepthTesting()
{
   glDisable(GL_DEPTH_TEST);
}

void
OpenGLRendererAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
   glViewport(x, y, width, height);
}

void
OpenGLRendererAPI::SetClearColor(const glm::vec4& color)
{
   glClearColor(color.r, color.g, color.b, color.a);
}

void
OpenGLRendererAPI::Clear()
{
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void
OpenGLRendererAPI::DrawIndexed(const std::shared_ptr< VertexArray >& vertexArray,
                               uint32_t indexCount)
{
   auto count =
      static_cast< GLsizei >(indexCount ? indexCount : vertexArray->GetIndexBuffer()->GetCount());
   glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
}

void
OpenGLRendererAPI::MultiDrawElemsIndirect(uint32_t drawCount, size_t offset)
{
  // glMemoryBarrier(GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT);
   glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, (void*)offset, drawCount, 0);
}

void
OpenGLRendererAPI::DrawLines(uint32_t count)
{
   glDrawArrays(GL_LINES, 0, count * 2);
}

} // namespace shady::render::opengl