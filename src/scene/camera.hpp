#pragma once

#include <glm/glm.hpp>

namespace shady::scene {

enum class CameraType{
  orthographic,
  perspective
};

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

   const glm::vec3&
   GetLookAtVec() const;

   const glm::vec3&
   GetUpVec() const;

   virtual void
   MouseMovement(const glm::vec2& mouseMovement) = 0;

   virtual void
   MoveCamera(const glm::vec2& leftRightVec) = 0;

 protected:
   void
   UpdateViewMatrix();

 protected:
   glm::mat4 m_projectionMat = glm::mat4(1.0f);
   glm::mat4 m_viewMat = glm::mat4(1.0f);
   glm::mat4 m_viewProjectionMat = glm::mat4(1.0f);

   glm::vec3 m_position = glm::vec3(0.0f);
   glm::vec3 m_upVector = glm::vec3(0.0f, 1.0f, 0.0f);
   glm::vec3 m_rightVector = glm::vec3(1.0f, 0.0f, 0.0f);
   glm::vec3 m_lookAtDirection = glm::vec3(0.0f, 0.0f, 1.0f);
};
} // namespace shady::render