#pragma once

#include "texture.hpp"

namespace shady::render {

enum class FrameBufferType {
  SINGLE,
  CUBE
};

using FramebufferIDType = uint32_t;

class FrameBuffer
{
 public:
   virtual void
   Bind() = 0;

   virtual void
   Unbind() = 0;

   static std::shared_ptr< FrameBuffer >
   Create(const glm::ivec2& size, FrameBufferType type);

 protected:
   uint32_t m_width;
   uint32_t m_height;
   FramebufferIDType m_framebufferID;
   TextureIDType m_textureID;
   TextureHandleType m_textureHandle;
};

} // namespace shady::render
