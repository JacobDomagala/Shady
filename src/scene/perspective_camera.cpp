#include "scene/perspective_camera.hpp"

#include <glm/gtx/transform.hpp>

namespace shady::scene {

PerspectiveCamera::PerspectiveCamera(const glm::mat4& projection) : Camera(projection)
{
}

PerspectiveCamera::PerspectiveCamera(float fieldOfView, float aspectRation, float near, float far)
{
   m_projectionMat = glm::perspective(fieldOfView, aspectRation, near, far);
}

void
PerspectiveCamera::MouseMovement(const glm::vec2& mouseMovement)
{
   m_yaw += mouseMovement.x;
   m_pitch += mouseMovement.y;

   if (m_constrainPitch)
   {
      glm::clamp(m_pitch, -89.0f, 89.0f);
   }

   m_lookAtDirection.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
   m_lookAtDirection.y = sin(glm::radians(m_pitch));
   m_lookAtDirection.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
   m_lookAtDirection = glm::normalize(m_lookAtDirection);

   auto rightVector = glm::normalize(glm::cross(m_lookAtDirection, m_worldUp));
   m_upVector = glm::normalize(glm::cross(rightVector, m_lookAtDirection));

   UpdateViewMatrix();
}

} // namespace shady::scene