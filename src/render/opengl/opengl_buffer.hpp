#pragma once

#include "buffer.hpp"

namespace shady::render::opengl {

class OpenGLVertexBuffer : public VertexBuffer
{
 public:
   explicit OpenGLVertexBuffer(uint32_t size);
   explicit OpenGLVertexBuffer(float* vertices, uint32_t size);
   virtual ~OpenGLVertexBuffer();

   void
   Bind() const override;
   void
   Unbind() const override;

   void
   SetData(const void* data, uint32_t size) override;

   const BufferLayout&
   GetLayout() const override;
   void
   SetLayout(const BufferLayout& layout) override;

 private:
   uint32_t m_rendererID;
   BufferLayout m_layout;
};

class OpenGLIndexBuffer : public IndexBuffer
{
 public:
   OpenGLIndexBuffer(uint32_t* indices, uint32_t count);
   virtual ~OpenGLIndexBuffer();

   void
   Bind() const override;
   void
   Unbind() const override;

   uint32_t
   GetCount() const override;

 private:
   uint32_t m_rendererID;
   uint32_t m_count;
};

} // namespace shady::render::opengl