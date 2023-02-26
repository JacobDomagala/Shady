#include "scene/camera.hpp"
#include "trace/logger.hpp"

#include <glm/gtx/transform.hpp>

namespace shady::scene {

Camera::Camera(const glm::mat4& projection, const glm::vec3& position)
   : projectionMat_(projection), position_(position)
{
   UpdateViewMatrix();
}

void
Camera::SetProjection(const glm::mat4& projection)
{
   projectionMat_ = projection;
}

const glm::mat4&
Camera::GetProjection() const
{
   return projectionMat_;
}

void
Camera::SetView(const glm::mat4& view)
{
   const auto inversed = glm::inverse(view);

   rightVector_ = glm::normalize(inversed[0]);
   position_ = inversed[3];
   lookAtDirection_ = glm::vec3(inversed[1]) - position_;
   upVector_ = glm::normalize(glm::cross(rightVector_, glm::normalize(lookAtDirection_)));

   viewMat_ = view;

   UpdateViewProjection();
}

const glm::mat4&
Camera::GetView() const
{
   return viewMat_;
}

void
Camera::SetViewProjection(const glm::mat4& viewProjection)
{
   viewProjectionMat_ = viewProjection;
}

const glm::mat4&
Camera::GetViewProjection() const
{
   return viewProjectionMat_;
}

void
Camera::SetPosition(const glm::vec3& position)
{
   position_ = position;
   UpdateViewMatrix();
}

const glm::vec3&
Camera::GetPosition() const
{
   return position_;
}

const glm::vec3&
Camera::GetLookAtVec() const
{
   return lookAtDirection_;
}

const glm::vec3&
Camera::GetUpVec() const
{
   return upVector_;
}

const glm::vec3&
Camera::GetRightVec() const
{
   return rightVector_;
}

void
Camera::UpdateViewMatrix()
{
   viewMat_ = glm::lookAt(position_, position_ + lookAtDirection_, upVector_);
   UpdateViewProjection();
}

void
Camera::UpdateViewProjection()
{
   viewProjectionMat_ = projectionMat_ * viewMat_;
}

} // namespace shady::scene