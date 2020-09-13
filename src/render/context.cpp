#include "context.hpp"

#include "openGL/openGL_context.hpp"
#include "renderer.hpp"
#include "trace/logger.hpp"

namespace shady::render {

std::unique_ptr< Context >
Context::Create(GLFWwindow* window)
{
   switch (Renderer::GetAPI())
   {
      case RendererAPI::API::None: {
         trace::Logger::Fatal("Context::Create -> RendererAPI::None is currently not supported!");
         return nullptr;
      }
      break;

      case RendererAPI::API::OpenGL: {
         return std::make_unique< opengl::OpenGLContext >(window);
      }
      break;
   }

   trace::Logger::Fatal("Context::Create -> Unknown RendererAPI!");
   return nullptr;
}

} // namespace shady::render