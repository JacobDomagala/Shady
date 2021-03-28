#include "scene/perspective_camera.hpp"
#include "trace/logger.hpp"

#include <glm/gtx/transform.hpp>

namespace shady::scene {

PerspectiveCamera::PerspectiveCamera(const glm::mat4& projection) : Camera(projection)
{
}

PerspectiveCamera::PerspectiveCamera(float fieldOfView, float aspectRatio, float nearClip, float farClip)
{
   m_projectionMat = glm::perspective(fieldOfView, aspectRatio, nearClip, farClip);
}

void
PerspectiveCamera::MouseMovement(const glm::vec2& mouseMovement)
{
   m_yaw += mouseMovement.x;
   m_pitch += mouseMovement.y;

   if (m_constrainPitch)
   {
      m_pitch = glm::clamp(m_pitch, -89.0f, 89.0f);
   }

   m_lookAtDirection.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
   m_lookAtDirection.y = sin(glm::radians(m_pitch));
   m_lookAtDirection.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
   m_lookAtDirection = glm::normalize(m_lookAtDirection);

   m_rightVector = glm::normalize(glm::cross(m_lookAtDirection, m_worldUp));
   m_upVector = glm::normalize(glm::cross(m_rightVector, m_lookAtDirection));

   UpdateViewMatrix();
}

void
PerspectiveCamera::MoveCamera(const glm::vec2& leftRightVec)
{
   constexpr auto cameraSpeed = 0.5f;

   trace::Logger::Trace("Camera Pos:{} LookAtDir:{} RightVec:{}", m_position, m_lookAtDirection,
                        m_rightVector);
   m_position += cameraSpeed *(leftRightVec.y * m_lookAtDirection);
   m_position += cameraSpeed *(leftRightVec.x * m_rightVector);

   UpdateViewMatrix();
}

} // namespace shady::scene
