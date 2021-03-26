#include "opengl_framebuffer.hpp"
#include "trace/logger.hpp"

#include <glad/glad.h>

namespace shady::render::opengl {

OpenGLFramebuffer::OpenGLFramebuffer(const glm::ivec2& size, FrameBufferType type)
{
   m_width = size.x;
   m_height = size.y;

   if (type == FrameBufferType::SINGLE)
   {
      glCreateTextures(GL_TEXTURE_2D, 1, &m_textureID);
      // glTextureStorage2D(m_textureID, 0, internalFormat, m_width, m_height);

      //    glTextureSubImage2D(m_textureID, 0, 0, 0, m_width, m_height, dataFormat,
      //    GL_DEPTH_COMPONENT,
      //                     nullptr);

      glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_width, m_height, 0, GL_DEPTH_COMPONENT,
                   GL_FLOAT, NULL);

      glTextureParameteri(m_textureID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTextureParameteri(m_textureID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTextureParameteri(m_textureID, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTextureParameteri(m_textureID, GL_TEXTURE_WRAP_T, GL_REPEAT);

      GLfloat borderColor[] = {1.0f, 0.0f, 0.0f, 0.0f};

      glTextureParameterfv(m_textureID, GL_TEXTURE_BORDER_COLOR, borderColor);
      glTextureParameteri(m_textureID, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
      glTextureParameteri(m_textureID, GL_TEXTURE_COMPARE_FUNC, GL_LESS);

      glCreateFramebuffers(1, &m_framebufferID);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_textureID, 0);
     // glDrawBuffer(GL_NONE);
     // glReadBuffer(GL_NONE);
//      glBindFramebuffer(GL_FRAMEBUFFER, 0);

      assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

      glBindFramebuffer(GL_FRAMEBUFFER, 0);
   }
   else
   {
      glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_textureID);

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
      // glDrawBuffer(GL_NONE);
      // glReadBuffer(GL_NONE);
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
   }

   //m_textureHandle = glGetTextureHandleARB(m_textureID);

   trace::Logger::Debug("Created {} framebuffer! Width:{} Height:{}",
                        type == FrameBufferType::SINGLE ? "single" : "cube", m_width, m_height);
}

void
OpenGLFramebuffer::Bind()
{
   glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferID);
   glBindTextureUnit(0, m_textureID);
}

void
OpenGLFramebuffer::Unbind()
{
   glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


} // namespace shady::render::opengl
