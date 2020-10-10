#include "framebuffer.hpp"
#include "opengl/opengl_framebuffer.hpp"
#include "renderer.hpp"

namespace shady::render {

std::shared_ptr< FrameBuffer >
FrameBuffer::Create(const glm::ivec2& size)
{
   switch (Renderer::GetAPI())
   {
      case RendererAPI::API::None: {
         trace::Logger::Fatal(
            "FrameBuffer::Create() -> RendererAPI::None is currently not supported!");
         return nullptr;
      }
      break;

      case RendererAPI::API::OpenGL: {
         return std::make_shared< opengl::OpenGLFramebuffer >(size);
      }
      break;
   }

   trace::Logger::Fatal("FrameBuffer::Create() -> Unknown RendererAPI!");
   return nullptr;
}


} // namespace shady::render