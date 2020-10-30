#pragma once

#include "buffer.hpp"

namespace shady::render::opengl {

class OpenGLVertexBuffer : public VertexBuffer
{
 public:
   explicit OpenGLVertexBuffer(size_t size);
   explicit OpenGLVertexBuffer(float* vertices, size_t size);
   ~OpenGLVertexBuffer() override;

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
   OpenGLIndexBuffer(uint32_t count);
   OpenGLIndexBuffer(uint32_t* indices, uint32_t count);
   ~OpenGLIndexBuffer() override;

   void
   Bind() const override;
   void
   Unbind() const override;

   void
   SetData(const void* data, size_t size) override;

   uint32_t
   GetCount() const override;

 private:
   uint32_t m_rendererID;
   uint32_t m_count;
};

class OpenGLDrawIndirectBuffer : public DrawIndirectBuffer
{
 public:
   OpenGLDrawIndirectBuffer();
   ~OpenGLDrawIndirectBuffer() override;

   void
   Bind() const override;
   void
   Unbind() const override;

   void
   SetData(const void* data, size_t size) override;

 private:
   uint32_t m_rendererID;
};

class OpenGLShaderStorageBuffer : public ShaderStorageBuffer
{
 public:
   OpenGLShaderStorageBuffer();
   ~OpenGLShaderStorageBuffer() override;

   void
   Bind() const override;
   void
   Unbind() const override;

   void
   SetData(const void* data, size_t size) override;

 private:
   uint32_t m_rendererID;
};

} // namespace shady::render::opengl