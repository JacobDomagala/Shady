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
   m_viewMat = view;
   UpdateViewMatrix();
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
   m_viewProjectionMat = m_projectionMat * m_viewMat;

   //trace::Logger::Info("Camera position = {}, upVec = {}, rightVec = {}, lookAt = {}", m_position,
   //                    m_upVector, m_rightVector, m_lookAtDirection);
}

} // namespace shady::scene