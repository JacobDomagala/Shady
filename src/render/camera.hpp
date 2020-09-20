#pragma once

#include <glm/glm.hpp>

namespace shady::render {


class Camera
{
 public:
   Camera() = default;
   Camera(const glm::mat4& projection);
   virtual ~Camera() = default;

   void
   SetProjection(const glm::mat4&);
   const glm::mat4&
   GetProjection() const;

   void
   SetView(const glm::mat4&);
   const glm::mat4&
   GetView() const;

   void
   SetViewProjection(const glm::mat4&);
   const glm::mat4&
   GetViewProjection() const;

   void
   SetPosition(const glm::vec3&);
   const glm::vec3&
   GetPosition() const;

 protected:
   void
   UpdateViewMatrix();

 protected:
   glm::mat4 m_projectionMat = glm::mat4(1.0f);
   glm::mat4 m_viewMat = glm::mat4(1.0f);
   glm::mat4 m_viewProjectionMat = glm::mat4(1.0f);

   glm::vec3 m_position = glm::vec3(0.0f);
   glm::vec3 m_upVector = glm::vec3(0.0f, 1.0f, 0.0f);
   glm::vec3 m_lookAtDirection = glm::vec3(0.0f, 0.0f, 1.0f);
};
} // namespace shady::render