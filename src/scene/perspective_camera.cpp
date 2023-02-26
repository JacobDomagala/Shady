#include "scene/perspective_camera.hpp"
#include "trace/logger.hpp"

#include <glm/gtx/transform.hpp>

namespace shady::scene {

PerspectiveCamera::PerspectiveCamera(const glm::mat4& projection, const glm::vec3& position)
   : Camera(projection, position)
{
}

PerspectiveCamera::PerspectiveCamera(float fieldOfView, float aspectRatio, float nearClip,
                                     float farClip, const glm::vec3& position)
   : Camera(glm::perspective(fieldOfView, aspectRatio, nearClip, farClip), position)
{
}

void
PerspectiveCamera::MouseMovement(const glm::vec2& mouseMovement)
{
   yaw_ += mouseMovement.x;
   pitch_ += mouseMovement.y;

   if (constrainPitch_)
   {
      pitch_ = glm::clamp(pitch_, -89.0f, 89.0f);
   }

   lookAtDirection_.x = -glm::cos(glm::radians(yaw_)) * glm::cos(glm::radians(pitch_));
   lookAtDirection_.y = glm::sin(glm::radians(pitch_));
   lookAtDirection_.z = -glm::sin(glm::radians(yaw_)) * glm::cos(glm::radians(pitch_));
   lookAtDirection_ = glm::normalize(lookAtDirection_);

   rightVector_ = glm::normalize(glm::cross(lookAtDirection_, worldUp_));
   upVector_ = glm::normalize(glm::cross(rightVector_, lookAtDirection_));

   UpdateViewMatrix();
}

void
PerspectiveCamera::MoveCamera(const glm::vec2& leftRightVec)
{
   constexpr auto cameraSpeed = 0.5f;

   trace::Logger::Trace("Camera Pos:{} LookAtDir:{} RightVec:{}", position_, lookAtDirection_,
                        rightVector_);
   position_ += cameraSpeed * (leftRightVec.y * lookAtDirection_);
   position_ += cameraSpeed * (leftRightVec.x * rightVector_);

   UpdateViewMatrix();
}

void
PerspectiveCamera::RotateCamera(float /*angle*/, const glm::vec3& /*axis*/)
{
   lookAtDirection_ = glm::normalize(lookAtDirection_);

   rightVector_ = glm::normalize(glm::cross(lookAtDirection_, worldUp_));
   upVector_ = glm::normalize(glm::cross(rightVector_, lookAtDirection_));
   UpdateViewMatrix();
}

} // namespace shady::scene
