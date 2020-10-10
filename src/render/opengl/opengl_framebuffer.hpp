#pragma once

#include "framebuffer.hpp"

#include <glm/glm.hpp>

namespace shady::render::opengl {

class OpenGLFramebuffer : public FrameBuffer
{
 public:
   OpenGLFramebuffer(const glm::ivec2& size);

   void
   Bind() override;
   void
   Unbind() override;

 private:
};

} // namespace shady::render::opengl