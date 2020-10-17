#include "scene/light.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace shady::scene {

Light::Light(const glm::vec3& position, const glm::vec3& color, LightType type)
   : m_position(position), m_color(color)
{
   glm::mat4 projectionMatrix;

   switch (type)
   {
      case LightType::DIRECTIONAL_LIGHT: {
         projectionMatrix = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, -10.0f, 10.0f);
      }
      case LightType::POINT_LIGHT: {
         projectionMatrix = glm::perspective(60.0f, 1.0f, 0.1f, 100.0f);
      }
      case LightType::SPOTLIGHT: {
         projectionMatrix = glm::perspective(60.0f, 1.0f, 0.1f, 100.0f);
      }
   }

   glm::mat4 viewMatrix = glm::lookAt(position, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

   m_biasMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.5f))
                  * glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));

   // m_lightSpaceMatrix = projectionMatrix * viewMatrix * m_modelMatrix;
   // m_shadowMatrix = m_biasMatrix * m_lightSpaceMatrix;

   m_shadowBuffer = render::FrameBuffer::Create({m_shadowTextureWidth, m_shadowTextureHeight});
}

void
Light::BeginRenderToLightmap()
{
   /*glUseProgram(programID);
   glUniformMatrix4fv(glGetUniformLocation(programID, "lightSpaceMatrix"), 1, GL_FALSE,
                      glm::value_ptr(lightSpaceMatrix));

   glViewport(0, 0, shadowTextureWidth, shadowTextureHeight);

   glBindFramebuffer(GL_FRAMEBUFFER, shadowTexture.frameBufferID);
   glClear(GL_DEPTH_BUFFER_BIT);*/

   m_shadowBuffer->Bind();
}

void
Light::EndRenderToLightmap()
{
   m_shadowBuffer->Unbind();
}

void
Light::BindLightMap()
{
   m_shadowBuffer->Bind();
}

const glm::mat4&
Light::GetLightSpaceMat() const
{
   return m_lightSpaceMatrix;
}

} // namespace shady::scene