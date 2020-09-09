#include "buffer.hpp"
#include "renderer.hpp"
#include "trace/logger.hpp"

namespace shady::render {

std::shared_ptr<VertexBuffer>
VertexBuffer::Create(uint32_t size)
{
  switch (Renderer::GetAPI()) {
    case RendererAPI::API::None:
      trace::Logger::Fatal("RendererAPI::None is currently not supported!");
      return nullptr;
      // case RendererAPI::API::OpenGL:  return std::make_shared<OpenGLVertexBuffer>(size);

    default: {
      trace::Logger::Fatal("Unknown RendererAPI!");
      return nullptr;
    }
  }
}

std::shared_ptr<VertexBuffer>
VertexBuffer::Create(float *vertices, uint32_t size)
{
  switch (Renderer::GetAPI()) {
    case RendererAPI::API::None:
      trace::Logger::Fatal("RendererAPI::None is currently not supported!");
      return nullptr;
      // case RendererAPI::API::OpenGL:  return std::make_shared<OpenGLVertexBuffer>(vertices,
      // size);

    default: {
      trace::Logger::Fatal("Unknown RendererAPI!");
      return nullptr;
    }
  }
}

std::shared_ptr<IndexBuffer>
IndexBuffer::Create(uint32_t *indices, uint32_t size)
{
  switch (Renderer::GetAPI()) {
    case RendererAPI::API::None:
      trace::Logger::Fatal("RendererAPI::None is currently not supported!");
      return nullptr;
      // case RendererAPI::API::OpenGL:  return std::make_shared<OpenGLIndexBuffer>(indices, size);
    default: {

      trace::Logger::Fatal("Unknown RendererAPI!");
      return nullptr;
    }
  }
}

}// namespace shady::render