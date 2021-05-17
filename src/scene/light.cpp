#include "scene/light.hpp"
#include "trace/logger.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace shady::scene {

Light::Light(const glm::vec3& position, const glm::vec3& color, LightType type)
   : m_position(position), m_color(color)
{
   // auto buffer_type = render::FrameBufferType::SINGLE;
   switch (type)
   {
      case LightType::DIRECTIONAL_LIGHT: {
         m_projectionMatrix = glm::ortho(-200.0f, 200.0f, -200.0f, 200.0f, 1.0f, 500.0f);
         // m_projectionMatrix = glm::perspective(70.0f, 16.0f / 9.0f, 0.1f, 500.0f);
      }
      break;

      case LightType::POINT_LIGHT: {
         m_projectionMatrix = glm::perspective(60.0f, 1.0f, 0.1f, 100.0f);
         // buffer_type = render::FrameBufferType::CUBE;
      }
      break;

      case LightType::SPOTLIGHT: {
         m_projectionMatrix = glm::perspective(60.0f, 1.0f, 0.1f, 100.0f);
      }
      break;
   }

   // m_shadowBuffer = render::FrameBuffer::Create({m_shadowTextureWidth, m_shadowTextureHeight},
   // buffer_type);
   m_biasMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.5f))
                  * glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));
   m_lookAt = glm::vec3(0.0f, -1.0f, 0.0f);
   m_upVec = glm::vec3(0.0f, 0.0f, 0.5f);
   // m_shadowBuffer->MakeTextureResident();

   UpdateViewProjection();
}

void
Light::BeginRenderToLightmap()
{
   m_viewMatrix = glm::lookAt(m_position, m_lookAt, glm::vec3(0.0f, 0.0f, 0.5f));
   m_lightSpaceMatrix = m_projectionMatrix * m_viewMatrix;
   m_shadowMatrix = m_biasMatrix * m_lightSpaceMatrix;


   // m_shadowBuffer->Bind();
   // render::RenderCommand::Clear();
   // m_shadowBuffer->MakeTextureNonResident();
}

void
Light::EndRenderToLightmap()
{
   // m_shadowBuffer->MakeTextureResident();
   // m_shadowBuffer->Unbind();
}

// render::TextureHandleType
// Light::GetDepthMapHandle()
//{
//   return m_shadowBuffer->GetDepthMapHandle();
//   // m_shadowBuffer->Bind();
//   // m_shadowBuffer->MakeResident();
//}
//
// render::TextureIDType
// Light::GetDepthMapID()
//{
//   return m_shadowBuffer->GetDepthMapID();
//}

void
Light::BindLightMap(uint32_t /*slot*/)
{
   // m_shadowBuffer->BindTexture(slot);
}

const glm::mat4&
Light::GetLightSpaceMat() const
{
   return m_lightSpaceMatrix;
}

const glm::mat4&
Light::GetViewMat() const
{
   return m_viewMatrix;
}

glm::vec3
Light::GetPosition() const
{
   return m_position;
}

glm::vec3
Light::GetLookAt() const
{
   return m_lookAt;
}

glm::vec3
Light::GetColor() const
{
   return m_color;
}

void
Light::SetColor(const glm::vec3& new_color)
{
   m_color = new_color;
}

glm::tvec2< uint32_t >
Light::GetLightmapSize() const
{
   return {m_shadowTextureWidth, m_shadowTextureHeight};
}

void
Light::MoveBy(const glm::vec3& moveBy)
{
   m_position += moveBy;

   UpdateViewProjection();
   // trace::Logger::Info("Light position {}", m_position);
}

void
Light::UpdateViewProjection()
{
   m_viewMatrix = glm::lookAt(m_position, m_lookAt, m_upVec);
   m_lightSpaceMatrix = m_projectionMatrix * m_viewMatrix;
   m_shadowMatrix = m_biasMatrix * m_lightSpaceMatrix;
}

} // namespace shady::scene
