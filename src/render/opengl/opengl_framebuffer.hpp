#pragma once

#include "render/framebuffer.hpp"

#include <glm/glm.hpp>

namespace shady::render::opengl {

class OpenGLFramebuffer : public FrameBuffer
{
 public:
   OpenGLFramebuffer(const glm::ivec2& size, FrameBufferType type);

   void
   Bind() override;
   void
   Unbind() override;
};

} // namespace shady::render::opengl