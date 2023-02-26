#include "scene/light.hpp"
#include "trace/logger.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace shady::scene {

Light::Light(const glm::vec3& position, const glm::vec3& color, LightType type)
   : position_(position), color_(color)
{
   // auto buffer_type = render::FrameBufferType::SINGLE;
   switch (type)
   {
      case LightType::DIRECTIONAL_LIGHT: {
         projectionMatrix_ = glm::ortho(-200.0f, 200.0f, -200.0f, 200.0f, 1.0f, 500.0f);
         // projectionMatrix_ = glm::perspective(70.0f, 16.0f / 9.0f, 0.1f, 500.0f);
      }
      break;

      case LightType::POINT_LIGHT:
      case LightType::SPOTLIGHT: {
         projectionMatrix_ = glm::perspective(60.0f, 1.0f, 0.1f, 100.0f);
         // buffer_type = render::FrameBufferType::CUBE;
      }
      break;
   }

   // m_shadowBuffer = render::FrameBuffer::Create({shadowTextureWidth_, shadowTextureHeight_},
   // buffer_type);
   biasMatrix_ = glm::translate(glm::mat4(1.0f), glm::vec3(0.5f))
                  * glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));
   lookAt_ = glm::vec3(0.0f, -1.0f, 0.0f);
   upVec_ = glm::vec3(0.0f, 0.0f, 0.5f);
   // m_shadowBuffer->MakeTextureResident();

   UpdateViewProjection();
}

const glm::mat4&
Light::GetLightSpaceMat() const
{
   return lightSpaceMatrix_;
}

const glm::mat4&
Light::GetViewMat() const
{
   return viewMatrix_;
}

glm::vec3
Light::GetPosition() const
{
   return position_;
}

glm::vec3
Light::GetLookAt() const
{
   return lookAt_;
}

glm::vec3
Light::GetColor() const
{
   return color_;
}

void
Light::SetColor(const glm::vec3& new_color)
{
   color_ = new_color;
}

glm::tvec2< uint32_t >
Light::GetLightmapSize() const
{
   return {shadowTextureWidth_, shadowTextureHeight_};
}

void
Light::MoveBy(const glm::vec3& moveBy)
{
   position_ += moveBy;

   UpdateViewProjection();
   // trace::Logger::Info("Light position {}", position_);
}

void
Light::UpdateViewProjection()
{
   viewMatrix_ = glm::lookAt(position_, lookAt_, upVec_);
   lightSpaceMatrix_ = projectionMatrix_ * viewMatrix_;
   shadowMatrix_ = biasMatrix_ * lightSpaceMatrix_;
}

} // namespace shady::scene
