
#include "renderer_api.hpp"
#include "openGL/openGL_renderer_api.hpp"
#include "trace/logger.hpp"

namespace shady::render {

std::unique_ptr< RendererAPI >
RendererAPI::Create()
{
   switch (s_API)
   {
      case RendererAPI::API::None: {
         trace::Logger::Fatal(
            "RendererAPI::Create -> RendererAPI::None is currently not supported!");
         return nullptr;
      }
      break;

      case RendererAPI::API::OpenGL: {
         return std::make_unique< opengl::OpenGLRendererAPI >();
      }
      break;
   }

   trace::Logger::Fatal("RendererAPI::Create -> Unknown RendererAPI!");
   return nullptr;
}

RendererAPI::API
RendererAPI::GetAPI()
{
   return s_API;
}
} // namespace shady::render