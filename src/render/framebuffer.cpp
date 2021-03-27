#include "framebuffer.hpp"
#include "helpers.hpp"
#include "opengl/opengl_framebuffer.hpp"
#include "renderer.hpp"
#include "trace/logger.hpp"

namespace shady::render {

TextureHandleType
FrameBuffer::GetDepthMapHandle()
{
   return m_textureHandle;
}

TextureIDType
FrameBuffer::GetDepthMapID()
{
   return m_textureID;
}

std::shared_ptr< FrameBuffer >
FrameBuffer::Create(const glm::ivec2& size, FrameBufferType type, FramebufferAttachment attachment)
{
   return CreateSharedWrapper< opengl::OpenGLFramebuffer, FrameBuffer >(size, type, attachment);
}

} // namespace shady::render
