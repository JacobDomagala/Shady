#include "opengl_buffer.hpp"
#include "utils/assert.hpp"

#include <fmt/format.h>
#include <glad/glad.h>


namespace shady::render::opengl {

/**************************************************************************************************
 *************************************** VERTEX BUFFER ********************************************
 *************************************************************************************************/
OpenGLVertexBuffer::OpenGLVertexBuffer(size_t sizeInBytes)
{
   glCreateBuffers(1, &m_rendererID);
   glBindBuffer(GL_ARRAY_BUFFER, m_rendererID);
   glBufferData(GL_ARRAY_BUFFER, sizeInBytes, nullptr, GL_DYNAMIC_DRAW);
}

OpenGLVertexBuffer::OpenGLVertexBuffer(float* vertices, size_t sizeInBytes)
{
   glCreateBuffers(1, &m_rendererID);
   glBindBuffer(GL_ARRAY_BUFFER, m_rendererID);
   glBufferData(GL_ARRAY_BUFFER, sizeInBytes, vertices, GL_STATIC_DRAW);
}

OpenGLVertexBuffer::OpenGLVertexBuffer()
{
   glCreateBuffers(1, &m_rendererID);
   glBindBuffer(GL_ARRAY_BUFFER, m_rendererID);
}

OpenGLVertexBuffer::~OpenGLVertexBuffer()
{
   glDeleteBuffers(1, &m_rendererID);
}

void
OpenGLVertexBuffer::Bind() const
{
   glBindBuffer(GL_ARRAY_BUFFER, m_rendererID);
}

void
OpenGLVertexBuffer::Unbind() const
{
   glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void
OpenGLVertexBuffer::SetData(const void* data, size_t size, size_t offsetInBytes)
{
   glBindBuffer(GL_ARRAY_BUFFER, m_rendererID);
   glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
}

const BufferLayout&
OpenGLVertexBuffer::GetLayout() const
{
   return m_layout;
}

void
OpenGLVertexBuffer::SetLayout(const BufferLayout& layout)
{
   m_layout = layout;
}

/**************************************************************************************************
 ************************************* MAPPED VERTEX BUFFER ***************************************
 *************************************************************************************************/
OpenGLMappedVertexBuffer::OpenGLMappedVertexBuffer(size_t size)
{
   const GLbitfield flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
   glBufferStorage(GL_ARRAY_BUFFER, size, nullptr, flags);
   m_baseMemPtr = reinterpret_cast< uint8_t* >(glMapBufferRange(GL_ARRAY_BUFFER, 0, size, flags));
}

void
OpenGLMappedVertexBuffer::SetData(const void* data, size_t size, size_t offset)
{
   void* dst = (unsigned char*)m_baseMemPtr + offset;
   memcpy(dst, data, size);
}

/**************************************************************************************************
 *************************************** INDEX BUFFER *********************************************
 *************************************************************************************************/
OpenGLIndexBuffer::OpenGLIndexBuffer(uint32_t count) : m_count(count)
{
   glCreateBuffers(1, &m_rendererID);

   // GL_ELEMENT_ARRAY_BUFFER is not valid without an actively bound VAO
   // Binding with GL_ARRAY_BUFFER allows the data to be loaded regardless of VAO state.
   glBindBuffer(GL_ARRAY_BUFFER, m_rendererID);
   glBufferData(GL_ARRAY_BUFFER, count * sizeof(uint32_t), nullptr, GL_DYNAMIC_DRAW);
}

OpenGLIndexBuffer::OpenGLIndexBuffer(uint32_t* indices, uint32_t count) : m_count(count)
{
   glCreateBuffers(1, &m_rendererID);

   // GL_ELEMENT_ARRAY_BUFFER is not valid without an actively bound VAO
   // Binding with GL_ARRAY_BUFFER allows the data to be loaded regardless of VAO state.
   glBindBuffer(GL_ARRAY_BUFFER, m_rendererID);
   glBufferData(GL_ARRAY_BUFFER, count * sizeof(uint32_t), indices, GL_STATIC_DRAW);
}

OpenGLIndexBuffer::~OpenGLIndexBuffer()
{
   glDeleteBuffers(1, &m_rendererID);
}

void
OpenGLIndexBuffer::Bind() const
{
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_rendererID);
}

void
OpenGLIndexBuffer::Unbind() const
{
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void
OpenGLIndexBuffer::SetData(const void* data, size_t size)
{
   glBindBuffer(GL_ARRAY_BUFFER, m_rendererID);
   glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
}

uint32_t
OpenGLIndexBuffer::GetCount() const
{
   return m_count;
}

/**************************************************************************************************
 ************************************* SHADER STORAGE BUFFER **************************************
 *************************************************************************************************/
OpenGLStorageBuffer::OpenGLStorageBuffer(BufferType type, size_t size) : m_bufferLock(true)
{
   switch (type)
   {
      case BufferType::ShaderStorage: {
         m_type = GL_SHADER_STORAGE_BUFFER;
      }
      break;

      case BufferType::DrawIndirect: {
         m_type = GL_DRAW_INDIRECT_BUFFER;
      }
      break;
   }

   glGenBuffers(1, &m_rendererID);

   const GLbitfield mapFlags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
   const GLbitfield createFlags = mapFlags | GL_DYNAMIC_STORAGE_BIT;

   const auto tripleBuffer = 3 * size;

   glBindBuffer(m_type, m_rendererID);
   glBufferStorage(m_type, tripleBuffer, nullptr, createFlags);

   m_baseMemPtr = reinterpret_cast< uint8_t* >(glMapBufferRange(m_type, 0, tripleBuffer, mapFlags));

   m_capacity = tripleBuffer;
}

OpenGLStorageBuffer::~OpenGLStorageBuffer()
{
   glDeleteBuffers(1, &m_rendererID);
}

void
OpenGLStorageBuffer::Bind() const
{
   glBindBuffer(m_type, m_rendererID);
}

void
OpenGLStorageBuffer::Unbind() const
{
   glBindBuffer(m_type, 0);
}

void
OpenGLStorageBuffer::SetData(const void* data, size_t size)
{
   utils::Assert(size <= m_capacity,
                 fmt::format("Attempting to copy more data than allocated! Size:{} Capacity:{}",
                             size, m_capacity));

   if (m_currentHead + size > m_capacity)
   {
      m_currentHead = 0;
   }

   m_bufferLock.WaitForLockedRange(m_currentHead, size);

   void* dst = (unsigned char*)m_baseMemPtr + m_currentHead;
   memcpy(dst, data, size);
}

void
OpenGLStorageBuffer::BindBufferRange(uint32_t index, size_t size)
{
   // Should only be used by GL_SHADER_STORAGE_BUFFER
   glBindBufferRange(m_type, index, m_rendererID, m_currentHead, size);
}

size_t
OpenGLStorageBuffer::GetOffset() const
{
   return m_currentHead;
}

void
OpenGLStorageBuffer::OnUsageComplete(size_t usedBufferSize)
{
   m_bufferLock.LockRange(m_currentHead, usedBufferSize);
   m_currentHead = (m_currentHead + usedBufferSize) % m_capacity;
}

} // namespace shady::render::opengl
