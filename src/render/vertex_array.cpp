#include "vertex_array.hpp"
#include "renderer.hpp"
#include "trace/logger.hpp"

namespace shady::render {

std::shared_ptr<VertexArray>
VertexArray::Create()
{
  switch (Renderer::GetAPI()) {
    case RendererAPI::API::None: {
      trace::Logger::Fatal("RendererAPI::None is currently not supported!");
      return nullptr;
    } break;
    // case RendererAPI::API::OpenGL:  return std::make_sharec<OpenGLVertexArray>();

    default: {
      trace::Logger::Fatal("Unknown RendererAPI!");
      return nullptr;
    }
  }
}

}// namespace shady::render