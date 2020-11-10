#pragma once

#include <memory>
#include <string>
#include <vector>


namespace shady::render {

enum class ShaderDataType
{
   Float,
   Float2,
   Float3,
   Float4,
   Mat3,
   Mat4,
   Int,
   Int2,
   Int3,
   Int4,
   Bool
};

/**************************************************************************************************
 *************************************** BUFFER ELEMENT *******************************************
 *************************************************************************************************/
struct BufferElement
{
   std::string m_name;
   ShaderDataType m_type;
   uint32_t m_size;
   size_t m_offset;
   bool m_normalized;
   uint32_t m_divisor;

   BufferElement() = default;
   BufferElement(ShaderDataType type, const std::string& name, uint32_t divisor = 0, bool normalized = false);

   uint32_t
   GetComponentCount() const;
};

/**************************************************************************************************
 *************************************** BUFFER LAYOUT ********************************************
 *************************************************************************************************/
class BufferLayout
{
 public:
   BufferLayout() = default;
   BufferLayout(const std::initializer_list< BufferElement >& elements);

   uint32_t
   GetStride() const;

   const std::vector< BufferElement >&
   GetElements() const;

   std::vector< BufferElement >::iterator
   begin();

   std::vector< BufferElement >::iterator
   end();

   std::vector< BufferElement >::const_iterator
   begin() const;

   std::vector< BufferElement >::const_iterator
   end() const;

 private:
   void
   CalculateOffsetsAndStride();

 private:
   std::vector< BufferElement > m_elements;
   uint32_t m_stride = 0;
};

/**************************************************************************************************
 *************************************** VERTEX BUFFER ********************************************
 *************************************************************************************************/
class VertexBuffer
{
 public:
   virtual ~VertexBuffer() = default;

   virtual void
   Bind() const = 0;
   virtual void
   Unbind() const = 0;

   virtual void
   SetData(const void* data, size_t size, size_t offsetInBytes = 0) = 0;

   virtual const BufferLayout&
   GetLayout() const = 0;
   virtual void
   SetLayout(const BufferLayout& layout) = 0;

   static std::shared_ptr< VertexBuffer >
   CreatePersistanceMap(size_t sizeInBytes);
   static std::shared_ptr< VertexBuffer >
   Create(size_t sizeInBytes);
   static std::shared_ptr< VertexBuffer >
   Create(float* vertices, size_t sizeInBytes);
};

/**************************************************************************************************
 *************************************** INDEX BUFFER *********************************************
 *************************************************************************************************/
class IndexBuffer
{
 public:
   virtual ~IndexBuffer() = default;

   virtual void
   Bind() const = 0;
   virtual void
   Unbind() const = 0;

   virtual void
   SetData(const void* data, size_t size) = 0;

   virtual uint32_t
   GetCount() const = 0;

   static std::shared_ptr< IndexBuffer >
   Create(uint32_t count);

   static std::shared_ptr< IndexBuffer >
   Create(uint32_t* indices, uint32_t count);
};

/**************************************************************************************************
 *************************************** STORAGE BUFFER *******************************************
 *************************************************************************************************/
enum class BufferType
{
   ShaderStorage,
   DrawIndirect
};

class StorageBuffer
{
 public:
   virtual ~StorageBuffer() = default;

   virtual void
   Bind() const = 0;
   virtual void
   Unbind() const = 0;

   virtual void
   SetData(const void* data, size_t size) = 0;

   virtual void
   BindBufferRange(uint32_t index, size_t atomCount) = 0;

   virtual void
   OnUsageComplete(size_t usedBufferSize) = 0;

   // Return current Head offset of storage buffer
   virtual size_t
   GetOffset() const = 0;

   static std::shared_ptr< StorageBuffer >
   Create(BufferType type, size_t size);
};

} // namespace shady::render