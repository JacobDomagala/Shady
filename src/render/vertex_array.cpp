#include "vertex_array.hpp"
#include "openGL/openGL_vertex_array.hpp"
#include "renderer.hpp"
#include "trace/logger.hpp"

namespace shady::render {

std::shared_ptr< VertexArray >
VertexArray::Create()
{
   switch (Renderer::GetAPI())
   {
      case RendererAPI::API::None: {
         trace::Logger::Fatal(
            "VertexArray::Create() -> RendererAPI::None is currently not supported!");
         return nullptr;
      }
      break;

      case RendererAPI::API::OpenGL: {
         return std::make_shared< openGL::OpenGLVertexArray >();
      }
      break;
   }

   trace::Logger::Fatal("VertexArray::Create() -> Unknown RendererAPI!");
   return nullptr;
}

} // namespace shady::render