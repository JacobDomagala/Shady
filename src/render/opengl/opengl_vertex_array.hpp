#pragma once

#include "vertex_array.hpp"

#include <memory>

namespace shady::render::opengl {

class OpenGLVertexArray : public VertexArray
{
 public:
   virtual ~OpenGLVertexArray() = default;

   void
   Bind() const override;

   void
   Unbind() const override;

   void
   AddVertexBuffer(const std::shared_ptr< VertexBuffer >& vertexBuffer) override;

   void
   SetIndexBuffer(const std::shared_ptr< IndexBuffer >& indexBuffer) override;

   const std::vector< std::shared_ptr< VertexBuffer > >&
   GetVertexBuffers() const override;

   const std::shared_ptr< IndexBuffer >&
   GetIndexBuffer() const override;

 private:
   uint32_t m_vertexArrayID;
   uint32_t m_vertexBufferIndex = 0;
   std::vector< std::shared_ptr< VertexBuffer > > m_vertexBuffers;
   std::shared_ptr< IndexBuffer > m_indexBuffer;
};

} // namespace shady::render::opengl
