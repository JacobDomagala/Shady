#pragma once

#include "buffer.hpp"

namespace shady::render::opengl {

class OpenGLVertexBuffer : public VertexBuffer
{
 public:
   explicit OpenGLVertexBuffer(size_t size);
   explicit OpenGLVertexBuffer(float* vertices, size_t size);
   virtual ~OpenGLVertexBuffer();

   void
   Bind() const override;
   void
   Unbind() const override;

   void
   SetData(const void* data, size_t size) override;

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
   OpenGLIndexBuffer(size_t count);
   OpenGLIndexBuffer(uint32_t* indices, size_t count);
   virtual ~OpenGLIndexBuffer();

   void
   Bind() const override;
   void
   Unbind() const override;

   void
   SetData(const void* data, size_t size) override;

   size_t
   GetCount() const override;

 private:
   uint32_t m_rendererID;
   size_t m_count;
};

} // namespace shady::render::opengl