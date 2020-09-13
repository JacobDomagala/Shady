#include "openGL_buffer.hpp"

#include <glad/glad.h>

namespace shady::render::opengl {

/**************************************************************************************************
 *************************************** VERTEX BUFFER ********************************************
 *************************************************************************************************/
OpenGLVertexBuffer::OpenGLVertexBuffer(uint32_t size)
{
   glCreateBuffers(1, &m_rendererID);
   glBindBuffer(GL_ARRAY_BUFFER, m_rendererID);
   glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
}

OpenGLVertexBuffer::OpenGLVertexBuffer(float* vertices, uint32_t size)
{
   glCreateBuffers(1, &m_rendererID);
   glBindBuffer(GL_ARRAY_BUFFER, m_rendererID);
   glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
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
OpenGLVertexBuffer::SetData(const void* data, uint32_t size)
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
 *************************************** INDEX BUFFER *********************************************
 *************************************************************************************************/
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

uint32_t
OpenGLIndexBuffer::GetCount() const
{
   return m_count;
}

} // namespace shady::render::opengl
