#include "opengl_framebuffer.hpp"
#include "glad/glad.h"

namespace shady::render::opengl {

OpenGLFramebuffer::OpenGLFramebuffer(const glm::ivec2& size)
{
   glGenFramebuffers(1, &m_framebufferHandle);
   glCreateTextures(GL_TEXTURE_2D,1, &m_textureID);

   glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_width, m_height, 0, GL_DEPTH_COMPONENT,
                GL_FLOAT, NULL);

   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

   GLfloat borderColor[] = {1.0f, 0.0f, 0.0f, 0.0f};

   glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LESS);

   glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferHandle);

   glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_textureID, 0);

   glDrawBuffer(GL_NONE);
   glReadBuffer(GL_NONE);
   glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void
OpenGLFramebuffer::Bind()
{
   glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferHandle);
}

void
OpenGLFramebuffer::Unbind()
{
   glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


} // namespace shady::render::opengl