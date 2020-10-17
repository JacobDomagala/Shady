#include "camera.hpp"

#include <glm/gtx/transform.hpp>

namespace shady::render {

Camera::Camera(const glm::mat4& projection) : m_projectionMat(projection)
{
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
}

const glm::mat4&
Camera::GetView() const
{
   return m_viewMat;
}

void
Camera::SetViewProjection(const glm::mat4& viewProjection)
{
   m_viewMat = viewProjection;
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

void
Camera::MoveBy(const glm::vec3& moveBy)
{
   m_position += moveBy;
}

void
Camera::UpdateViewMatrix()
{
   m_viewMat = glm::lookAt(m_position, m_position + m_lookAtDirection, m_upVector);
   m_viewProjectionMat = m_projectionMat * m_viewMat;
}

} // namespace shady::render