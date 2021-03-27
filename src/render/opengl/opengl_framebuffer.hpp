#pragma once

#include "render/framebuffer.hpp"

#include <glm/glm.hpp>

namespace shady::render::opengl {

class OpenGLFramebuffer : public FrameBuffer
{
 public:
   OpenGLFramebuffer(const glm::ivec2& size, FrameBufferType type, FramebufferAttachment attachment);

   void
   Bind() override;

   void
   Unbind() override;

   void
   MakeTextureResident() override;

   void
   MakeTextureNonResident() override;

   void
   BindTexture(uint32_t slot) override;
};

} // namespace shady::render::opengl
