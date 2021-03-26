#include "framebuffer.hpp"
#include "helpers.hpp"
#include "opengl/opengl_framebuffer.hpp"
#include "renderer.hpp"
#include "trace/logger.hpp"

namespace shady::render {

std::shared_ptr< FrameBuffer >
FrameBuffer::Create(const glm::ivec2& size, FrameBufferType type)
{
   return CreateSharedWrapper< opengl::OpenGLFramebuffer, FrameBuffer >(size, type);
}

} // namespace shady::render