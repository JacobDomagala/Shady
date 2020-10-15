#include "framebuffer.hpp"
#include "helpers.hpp"
#include "opengl/opengl_framebuffer.hpp"
#include "renderer.hpp"
#include "trace/logger.hpp"

namespace shady::render {

std::shared_ptr< FrameBuffer >
FrameBuffer::Create(const glm::ivec2& size)
{
   return CreateSharedWrapper< opengl::OpenGLFramebuffer, FrameBuffer >(size);
}


} // namespace shady::render