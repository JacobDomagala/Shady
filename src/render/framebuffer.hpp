#pragma once

#include "texture.hpp"

namespace shady::render {

class FrameBuffer
{
 public:
   virtual void
   Bind() = 0;
   virtual void
   Unbind() = 0;

   static std::shared_ptr< FrameBuffer >
   Create(const glm::ivec2& size);

 protected::
 uint32_t m_width;
 uint32_t m_height;
 uint32_t m_framebufferHandle;
 TextureHandleType m_textureHandle;
};

} // namespace shady::render