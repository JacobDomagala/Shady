#pragma once

#include "render/buffer.hpp"
#include "render/opengl/opengl_buffer_lock.hpp"

namespace shady::render::vulkan {

class VulkanVertexBuffer
{
 public:
   explicit VulkanVertexBuffer(size_t sizeInBytes);
   explicit VulkanVertexBuffer(float* vertices, size_t sizeInBytes);
   ~VulkanVertexBuffer();

   void
   Bind() const;
   void
   Unbind() const;

   void
   SetData(const void* data, size_t size, size_t offsetInBytes = 0);

   const BufferLayout&
   GetLayout() const;
   void
   SetLayout(const BufferLayout& layout);

 protected:
   // used by OpenGLMappedVertexBuffer
   VulkanVertexBuffer();

 protected:
   uint32_t m_rendererID;
   BufferLayout m_layout;
};


class VulkanIndexBuffer
{
 public:
   explicit VulkanIndexBuffer(uint32_t count);
   VulkanIndexBuffer(uint32_t* indices, uint32_t count);
   ~VulkanIndexBuffer();

   void
   Bind() const;
   void
   Unbind() const;

   void
   SetData(const void* data, size_t size);

   uint32_t
   GetCount() const;

 private:
   uint32_t m_rendererID;
   uint32_t m_count;
};



} // namespace shady::render::vulkan
