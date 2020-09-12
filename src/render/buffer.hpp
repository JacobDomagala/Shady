#pragma once

#include "trace/logger.hpp"

#include <vector>

namespace shady::render {

enum class ShaderDataType
{
  None = 0,
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

struct BufferElement
{
  std::string m_name;
  ShaderDataType m_type;
  uint32_t m_size;
  size_t m_offset;
  bool m_normalized;

  BufferElement() = default;
  BufferElement(ShaderDataType type, const std::string &name, bool normalized = false);

  uint32_t
  GetComponentCount() const;
};

class BufferLayout
{
public:
  BufferLayout() = default;
  BufferLayout(const std::initializer_list<BufferElement> &elements) : m_elements(elements);

  uint32_t
  GetStride() const;

  const std::vector<BufferElement> &
  GetElements() const;

  std::vector<BufferElement>::iterator
  begin();

  std::vector<BufferElement>::iterator
  end();

  std::vector<BufferElement>::const_iterator
  begin() const;

  std::vector<BufferElement>::const_iterator
  end() const;

private:
  void
  CalculateOffsetsAndStride();

private:
  std::vector<BufferElement> m_elements;
  uint32_t m_stride = 0;
};

class VertexBuffer
{
public:
  virtual ~VertexBuffer() = default;

  virtual void
  Bind() const = 0;
  virtual void
  Unbind() const = 0;

  virtual void
  SetData(const void *data, uint32_t size) = 0;

  virtual const BufferLayout &
  GetLayout() const = 0;
  virtual void
  SetLayout(const BufferLayout &layout) = 0;

  static std::shared_ptr<VertexBuffer>
  Create(uint32_t size);
  static std::shared_ptr<VertexBuffer>
  Create(float *vertices, uint32_t size);
};

class IndexBuffer
{
public:
  virtual ~IndexBuffer() = default;

  virtual void
  Bind() const = 0;
  virtual void
  Unbind() const = 0;

  virtual uint32_t
  GetCount() const = 0;

  static std::shared_ptr<IndexBuffer>
  Create(uint32_t *indices, uint32_t count);
};

}// namespace shady::render