#include "buffer.hpp"
#include "renderer.hpp"
#include "trace/logger.hpp"
#include "opengl/opengl_buffer.hpp"

namespace shady::render {

static uint32_t
ShaderDataTypeSize(ShaderDataType type)
{
   switch (type)
   {
      case ShaderDataType::Float:
         return 4;
      case ShaderDataType::Float2:
         return 4 * 2;
      case ShaderDataType::Float3:
         return 4 * 3;
      case ShaderDataType::Float4:
         return 4 * 4;
      case ShaderDataType::Mat3:
         return 4 * 3 * 3;
      case ShaderDataType::Mat4:
         return 4 * 4 * 4;
      case ShaderDataType::Int:
         return 4;
      case ShaderDataType::Int2:
         return 4 * 2;
      case ShaderDataType::Int3:
         return 4 * 3;
      case ShaderDataType::Int4:
         return 4 * 4;
      case ShaderDataType::Bool:
         return 1;
   }

   trace::Logger::Fatal("ShaderDataTypeSize -> Unknown ShaderDataType!");
   return 0;
}

/**************************************************************************************************
 *************************************** BUFFER ELEMENT *******************************************
 *************************************************************************************************/
BufferElement::BufferElement(ShaderDataType type, const std::string& name, bool normalized)
   : m_name(name),
     m_type(type),
     m_size(ShaderDataTypeSize(type)),
     m_offset(0),
     m_normalized(normalized)
{
}

uint32_t
BufferElement::GetComponentCount() const
{
   switch (m_type)
   {
      case ShaderDataType::Float:
         return 1;
      case ShaderDataType::Float2:
         return 2;
      case ShaderDataType::Float3:
         return 3;
      case ShaderDataType::Float4:
         return 4;
      case ShaderDataType::Mat3:
         return 3; // 3* float3
      case ShaderDataType::Mat4:
         return 4; // 4* float4
      case ShaderDataType::Int:
         return 1;
      case ShaderDataType::Int2:
         return 2;
      case ShaderDataType::Int3:
         return 3;
      case ShaderDataType::Int4:
         return 4;
      case ShaderDataType::Bool:
         return 1;
   }
   trace::Logger::Fatal("BufferElement::GetComponentCount -> Unknown ShaderDataType!");
   return 0;
}

/**************************************************************************************************
 *************************************** BUFFER LAYOUT ********************************************
 *************************************************************************************************/
BufferLayout::BufferLayout(const std::initializer_list< BufferElement >& elements)
   : m_elements(elements)
{
   CalculateOffsetsAndStride();
}

uint32_t
BufferLayout::GetStride() const
{
   return m_stride;
}
const std::vector< BufferElement >&
BufferLayout::GetElements() const
{
   return m_elements;
}

std::vector< BufferElement >::iterator
BufferLayout::begin()
{
   return m_elements.begin();
}
std::vector< BufferElement >::iterator
BufferLayout::end()
{
   return m_elements.end();
}
std::vector< BufferElement >::const_iterator
BufferLayout::begin() const
{
   return m_elements.begin();
}
std::vector< BufferElement >::const_iterator
BufferLayout::end() const
{
   return m_elements.end();
}

void
BufferLayout::CalculateOffsetsAndStride()
{
   size_t offset = 0;
   m_stride = 0;
   for (auto& element : m_elements)
   {
      element.m_offset = offset;
      offset += element.m_size;
      m_stride += element.m_size;
   }
}

/**************************************************************************************************
 *************************************** VERTEX BUFFER ********************************************
 *************************************************************************************************/
std::shared_ptr< VertexBuffer >
VertexBuffer::Create(uint32_t size)
{
   switch (Renderer::GetAPI())
   {
      case RendererAPI::API::None: {
         trace::Logger::Fatal(
            "VertexBuffer::Create -> RendererAPI::None is currently not supported!");
         return nullptr;
      }
      break;

      case RendererAPI::API::OpenGL: {
         return std::make_shared< opengl::OpenGLVertexBuffer >(size);
      }
      break;
   }

   trace::Logger::Fatal("VertexBuffer::Create -> Unknown RendererAPI!");
   return nullptr;
}

std::shared_ptr< VertexBuffer >
VertexBuffer::Create(float* vertices, uint32_t size)
{
   switch (Renderer::GetAPI())
   {
      case RendererAPI::API::None: {
         trace::Logger::Fatal(
            "VertexBuffer::Create -> RendererAPI::None is currently not supported!");
         return nullptr;
      }
      break;

      case RendererAPI::API::OpenGL: {
         return std::make_shared< opengl::OpenGLVertexBuffer >(vertices, size);
      }
      break;
   }

   trace::Logger::Fatal("VertexBuffer::Create -> Unknown RendererAPI!");
   return nullptr;
}

/**************************************************************************************************
 *************************************** INDEX BUFFER *********************************************
 *************************************************************************************************/
std::shared_ptr< IndexBuffer >
IndexBuffer::Create(uint32_t* indices, uint32_t size)
{
   switch (Renderer::GetAPI())
   {
      case RendererAPI::API::None: {
         trace::Logger::Fatal(
            "IndexBuffer::Create -> RendererAPI::None is currently not supported!");
         return nullptr;
      }
      break;

      case RendererAPI::API::OpenGL: {
         return std::make_shared< opengl::OpenGLIndexBuffer >(indices, size);
      }
      break;
   }

   trace::Logger::Fatal("IndexBuffer::Create -> Unknown RendererAPI!");
   return nullptr;
}

} // namespace shady::render