#include "opengl_framebuffer.hpp"
#include "render/render_command.hpp"
#include "trace/logger.hpp"

#include <glad/glad.h>

namespace shady::render::opengl {

OpenGLFramebuffer::OpenGLFramebuffer(const glm::tvec2<uint32_t>& size, FrameBufferType type,
                                     FramebufferAttachment attachment)
{
   m_width = size.x;
   m_height = size.y;

   if (type == FrameBufferType::SINGLE)
   {
      glGenFramebuffers(1, &m_framebufferID);
      glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferID);

      // For some reason we cannot call glCreateTextures here
      // It causes framebuffer to be incomplete!
      glGenTextures(1, &m_textureID);
      glBindTexture(GL_TEXTURE_2D, m_textureID);

      if (attachment == FramebufferAttachment::DEPTH)
      {
         glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_width, m_height, 0,
                      GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
         glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_textureID, 0);
         glDrawBuffer(GL_NONE);
         glReadBuffer(GL_NONE);
      }
      else if (attachment == FramebufferAttachment::COLOR)
      {
         glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
         glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_textureID, 0);
      }

      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
      GLfloat borderColor[] = {1.0, 1.0, 1.0, 1.0};
      glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

      glBindFramebuffer(GL_FRAMEBUFFER, 0);
   }
   else
   {
      glGenTextures(1, &m_textureID);
      glBindTexture(GL_TEXTURE_CUBE_MAP, m_textureID);

      for (unsigned int i = 0; i < 6; ++i)
      {
         glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, m_width, m_height,
                      0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
      }

      glTextureParameteri(m_textureID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTextureParameteri(m_textureID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTextureParameteri(m_textureID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTextureParameteri(m_textureID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTextureParameteri(m_textureID, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

      glGenFramebuffers(1, &m_framebufferID);
      glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferID);
      glNamedFramebufferTexture(m_framebufferID, GL_DEPTH_ATTACHMENT, m_textureID, 0);
      glDrawBuffer(GL_NONE);
      glReadBuffer(GL_NONE);

      glBindFramebuffer(GL_FRAMEBUFFER, 0);
   }

   m_textureHandle = glGetTextureHandleARB(m_textureID);

   trace::Logger::Debug("Created {} framebuffer! Width:{} Height:{}",
                        type == FrameBufferType::SINGLE ? "single" : "cube", m_width, m_height);

}

void
OpenGLFramebuffer::Bind()
{
   glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferID);
}

void
OpenGLFramebuffer::Unbind()
{
   glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void
OpenGLFramebuffer::MakeTextureResident()
{
   glMakeTextureHandleResidentARB(m_textureHandle);
}

void
OpenGLFramebuffer::MakeTextureNonResident()
{
   glMakeTextureHandleNonResidentARB(m_textureHandle);
}

void
OpenGLFramebuffer::BindTexture(uint32_t slot)
{
   glBindTextureUnit(slot, m_textureID);
}


} // namespace shady::render::opengl
