#pragma once

#include "texture.hpp"

namespace shady::render {

enum class FrameBufferType
{
   SINGLE,
   CUBE
};

enum class FramebufferAttachment {
  DEPTH,
  COLOR
};

using FramebufferIDType = uint32_t;

class FrameBuffer
{
 public:
   virtual void
   Bind() = 0;

   virtual void
   Unbind() = 0;

   virtual void
   MakeTextureResident() = 0;

   virtual void
   MakeTextureNonResident() = 0;

   virtual void
   BindTexture(uint32_t slot) = 0;

   TextureHandleType
   GetDepthMapHandle() const;

   TextureIDType
   GetDepthMapID() const;

   static std::shared_ptr< FrameBuffer >
   Create(const glm::ivec2& size, FrameBufferType type, FramebufferAttachment attachment = FramebufferAttachment::DEPTH);

 protected:
   uint32_t m_width;
   uint32_t m_height;
   FramebufferIDType m_framebufferID;
   TextureIDType m_textureID;
   TextureHandleType m_textureHandle;
};

} // namespace shady::render
