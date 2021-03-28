#pragma once

#include "render/buffer.hpp"
#include "render/opengl/opengl_buffer_lock.hpp"
namespace shady::render::opengl {

class OpenGLVertexBuffer : public VertexBuffer
{
 public:
   explicit OpenGLVertexBuffer(size_t sizeInBytes);
   explicit OpenGLVertexBuffer(float* vertices, size_t sizeInBytes);
   ~OpenGLVertexBuffer() override;

   void
   Bind() const override;
   void
   Unbind() const override;

   void
   SetData(const void* data, size_t size, size_t offsetInBytes = 0) override;

   const BufferLayout&
   GetLayout() const override;
   void
   SetLayout(const BufferLayout& layout) override;

 protected:
   // used by OpenGLMappedVertexBuffer
   OpenGLVertexBuffer();

 protected:
   uint32_t m_rendererID;
   BufferLayout m_layout;
};

class OpenGLMappedVertexBuffer : public OpenGLVertexBuffer
{
 public:
   explicit OpenGLMappedVertexBuffer(size_t size);

   void
   SetData(const void* data, size_t sizeInBytes, size_t offsetInBytes = 0) override;

 private:
   uint8_t* m_baseMemPtr = nullptr;
};

class OpenGLIndexBuffer : public IndexBuffer
{
 public:
   explicit OpenGLIndexBuffer(uint32_t count);
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

class OpenGLStorageBuffer : public StorageBuffer
{
 public:
   OpenGLStorageBuffer(BufferType type, size_t size);
   ~OpenGLStorageBuffer() override;

   void
   Bind() const override;
   void
   Unbind() const override;

   void
   SetData(const void* data, size_t size) override;

   void
   BindBufferRange(uint32_t index, size_t usedBufferSize) override;

   size_t
   GetOffset() const override;

   void
   OnUsageComplete(size_t usedBufferSize) override;

 private:
   uint32_t m_rendererID;
   uint8_t* m_baseMemPtr = nullptr;
   size_t m_currentHead = 0;
   size_t m_capacity = 0;
   GLenum m_type;
   OpenGLBufferLockManager m_bufferLock;
};

} // namespace shady::render::opengl
