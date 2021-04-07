#include "vulkan_buffer.hpp"
#include "utils/assert.hpp"

#include <fmt/format.h>
#include <glad/glad.h>


namespace shady::render::vulkan {

/**************************************************************************************************
 *************************************** VERTEX BUFFER ********************************************
 *************************************************************************************************/
VulkanVertexBuffer::VulkanVertexBuffer(size_t sizeInBytes)
{
   glCreateBuffers(1, &m_rendererID);
   glBindBuffer(GL_ARRAY_BUFFER, m_rendererID);
   glBufferData(GL_ARRAY_BUFFER, sizeInBytes, nullptr, GL_DYNAMIC_DRAW);
}

VulkanVertexBuffer::VulkanVertexBuffer(float* vertices, size_t sizeInBytes)
{
   glCreateBuffers(1, &m_rendererID);
   glBindBuffer(GL_ARRAY_BUFFER, m_rendererID);
   glBufferData(GL_ARRAY_BUFFER, sizeInBytes, vertices, GL_STATIC_DRAW);
}

VulkanVertexBuffer::VulkanVertexBuffer()
{
   glCreateBuffers(1, &m_rendererID);
   glBindBuffer(GL_ARRAY_BUFFER, m_rendererID);
}

VulkanVertexBuffer::~VulkanVertexBuffer()
{
   glDeleteBuffers(1, &m_rendererID);
}

void
VulkanVertexBuffer::Bind() const
{
   glBindBuffer(GL_ARRAY_BUFFER, m_rendererID);
}

void
VulkanVertexBuffer::Unbind() const
{
   glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void
VulkanVertexBuffer::SetData(const void* data, size_t size, size_t offsetInBytes)
{
   glBindBuffer(GL_ARRAY_BUFFER, m_rendererID);
   glBufferSubData(GL_ARRAY_BUFFER, offsetInBytes, size, data);
}

const BufferLayout&
VulkanVertexBuffer::GetLayout() const
{
   return m_layout;
}

void
VulkanVertexBuffer::SetLayout(const BufferLayout& layout)
{
   m_layout = layout;
}


/**************************************************************************************************
 *************************************** INDEX BUFFER *********************************************
 *************************************************************************************************/
VulkanIndexBuffer::VulkanIndexBuffer(uint32_t count) : m_count(count)
{
   glCreateBuffers(1, &m_rendererID);

   // GL_ELEMENT_ARRAY_BUFFER is not valid without an actively bound VAO
   // Binding with GL_ARRAY_BUFFER allows the data to be loaded regardless of VAO state.
   glBindBuffer(GL_ARRAY_BUFFER, m_rendererID);
   glBufferData(GL_ARRAY_BUFFER, count * sizeof(uint32_t), nullptr, GL_DYNAMIC_DRAW);
}

VulkanIndexBuffer::VulkanIndexBuffer(uint32_t* indices, uint32_t count) : m_count(count)
{
   glCreateBuffers(1, &m_rendererID);

   // GL_ELEMENT_ARRAY_BUFFER is not valid without an actively bound VAO
   // Binding with GL_ARRAY_BUFFER allows the data to be loaded regardless of VAO state.
   glBindBuffer(GL_ARRAY_BUFFER, m_rendererID);
   glBufferData(GL_ARRAY_BUFFER, count * sizeof(uint32_t), indices, GL_STATIC_DRAW);
}

VulkanIndexBuffer::~VulkanIndexBuffer()
{
   glDeleteBuffers(1, &m_rendererID);
}

void
VulkanIndexBuffer::Bind() const
{
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_rendererID);
}

void
VulkanIndexBuffer::Unbind() const
{
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void
VulkanIndexBuffer::SetData(const void* data, size_t size)
{
   glBindBuffer(GL_ARRAY_BUFFER, m_rendererID);
   glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
}

uint32_t
VulkanIndexBuffer::GetCount() const
{
   return m_count;
}


} // namespace shady::render::opengl
