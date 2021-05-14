#include "scene/camera.hpp"
#include "trace/logger.hpp"

#include <glm/gtx/transform.hpp>

namespace shady::scene {

Camera::Camera(const glm::mat4& projection) : m_projectionMat(projection)
{
   UpdateViewMatrix();
}

void
Camera::SetProjection(const glm::mat4& projection)
{
   m_projectionMat = projection;
}

const glm::mat4&
Camera::GetProjection() const
{
   return m_projectionMat;
}

void
Camera::SetView(const glm::mat4& view)
{
   const auto inversed = glm::inverse(view);

   m_rightVector = glm::normalize(inversed[0]);
   m_position = inversed[3];
   m_lookAtDirection = glm::vec3(inversed[1]) - m_position;
   m_upVector = glm::normalize(glm::cross(m_rightVector, glm::normalize(m_lookAtDirection)));

   m_viewMat = view;

   UpdateViewProjection();
}

const glm::mat4&
Camera::GetView() const
{
   return m_viewMat;
}

void
Camera::SetViewProjection(const glm::mat4& viewProjection)
{
   m_viewProjectionMat = viewProjection;
}

const glm::mat4&
Camera::GetViewProjection() const
{
   return m_viewProjectionMat;
}

void
Camera::SetPosition(const glm::vec3& position)
{
   m_position = position;
   UpdateViewMatrix();
}

const glm::vec3&
Camera::GetPosition() const
{
   return m_position;
}

const glm::vec3&
Camera::GetLookAtVec() const
{
   return m_lookAtDirection;
}

const glm::vec3&
Camera::GetUpVec() const
{
   return m_upVector;
}

void
Camera::UpdateViewMatrix()
{
   m_viewMat = glm::lookAt(m_position, m_position + m_lookAtDirection, m_upVector);
   UpdateViewProjection();
}

void
Camera::UpdateViewProjection()
{
   m_viewProjectionMat = m_projectionMat * m_viewMat;
}

} // namespace shady::scene