#include "openGL_vertex_array.hpp"

namespace shady::render::opengl {

void
OpenGLVertexArray::Bind() const
{
}

void
OpenGLVertexArray::Unbind() const
{
}

void
OpenGLVertexArray::AddVertexBuffer(const std::shared_ptr< VertexBuffer >& vertexBuffer)
{
}

void
OpenGLVertexArray::SetIndexBuffer(const std::shared_ptr< IndexBuffer >& indexBuffer)
{
}

const std::vector< std::shared_ptr< VertexBuffer > >&
OpenGLVertexArray::GetVertexBuffers() const
{
   return {};
}

const std::shared_ptr< IndexBuffer >&
OpenGLVertexArray::GetIndexBuffer() const
{
   return {};
}

} // namespace shady::render::opengl