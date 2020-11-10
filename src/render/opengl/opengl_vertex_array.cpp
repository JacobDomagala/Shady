#include "opengl_vertex_array.hpp"
#include "trace/logger.hpp"

#include <glad/glad.h>

namespace shady::render::opengl {

static GLenum
ShaderDataTypeToOpenGLBaseType(ShaderDataType type)
{
   switch (type)
   {
      case ShaderDataType::Float:
         return GL_FLOAT;
      case ShaderDataType::Float2:
         return GL_FLOAT;
      case ShaderDataType::Float3:
         return GL_FLOAT;
      case ShaderDataType::Float4:
         return GL_FLOAT;
      case ShaderDataType::Mat3:
         return GL_FLOAT;
      case ShaderDataType::Mat4:
         return GL_FLOAT;
      case ShaderDataType::Int:
         return GL_INT;
      case ShaderDataType::Int2:
         return GL_INT;
      case ShaderDataType::Int3:
         return GL_INT;
      case ShaderDataType::Int4:
         return GL_INT;
      case ShaderDataType::Bool:
         return GL_BOOL;
   }

   return 0;
}

OpenGLVertexArray::OpenGLVertexArray()
{
   glGenVertexArrays(1, &m_vertexArrayID);
}

void
OpenGLVertexArray::Bind() const
{
   glBindVertexArray(m_vertexArrayID);

   if (m_vertexBuffers.size())
   {
      for (auto& vbo : m_vertexBuffers)
      {
         vbo->Bind();
      }

      m_indexBuffer->Bind();
   }
}

void
OpenGLVertexArray::Unbind() const
{
   glBindVertexArray(0);
}

void
OpenGLVertexArray::AddVertexBuffer(const std::shared_ptr< VertexBuffer >& vertexBuffer)
{
   glBindVertexArray(m_vertexArrayID);
   vertexBuffer->Bind();

   const auto& layout = vertexBuffer->GetLayout();
   for (const auto& element : layout)
   {
      switch (element.m_type)
      {
         case ShaderDataType::Float:
         case ShaderDataType::Float2:
         case ShaderDataType::Float3:
         case ShaderDataType::Float4:
         case ShaderDataType::Int:
         case ShaderDataType::Int2:
         case ShaderDataType::Int3:
         case ShaderDataType::Int4:
         case ShaderDataType::Bool: {
            glEnableVertexAttribArray(m_vertexBufferIndex);
            glVertexAttribPointer(m_vertexBufferIndex, element.GetComponentCount(),
                                  ShaderDataTypeToOpenGLBaseType(element.m_type),
                                  element.m_normalized ? GL_TRUE : GL_FALSE, layout.GetStride(),
                                  (const void*)element.m_offset);
            glVertexAttribDivisor(m_vertexBufferIndex, element.m_divisor);
            m_vertexBufferIndex++;
            break;
         }
         case ShaderDataType::Mat3:
         case ShaderDataType::Mat4: {
            uint8_t count = element.GetComponentCount();
            for (uint8_t i = 0; i < count; i++)
            {
               glEnableVertexAttribArray(m_vertexBufferIndex);
               glVertexAttribPointer(m_vertexBufferIndex, count,
                                     ShaderDataTypeToOpenGLBaseType(element.m_type),
                                     element.m_normalized ? GL_TRUE : GL_FALSE, layout.GetStride(),
                                     (const void*)(sizeof(float) * count * i));
               glVertexAttribDivisor(m_vertexBufferIndex, 1);
               m_vertexBufferIndex++;
            }
            break;
         }
         default:
            break;
      }
   }

   m_vertexBuffers.push_back(vertexBuffer);
}

void
OpenGLVertexArray::SetIndexBuffer(const std::shared_ptr< IndexBuffer >& indexBuffer)
{
   m_indexBuffer = indexBuffer;
}

const std::vector< std::shared_ptr< VertexBuffer > >&
OpenGLVertexArray::GetVertexBuffers() const
{
   return m_vertexBuffers;
}

const std::shared_ptr< IndexBuffer >&
OpenGLVertexArray::GetIndexBuffer() const
{
   return m_indexBuffer;
}

} // namespace shady::render::opengl