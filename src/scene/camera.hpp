#pragma once

#include <glm/glm.hpp>

namespace shady::scene {

enum class CameraType
{
   orthographic,
   perspective
};

class Camera
{
 public:
   Camera() = delete;
   Camera(const Camera&) = delete;
   Camera(Camera&&) = delete;
   Camera&
   operator=(const Camera&) = delete;
   Camera&
   operator=(Camera&&) = delete;

   Camera(const glm::mat4& projection, const glm::vec3& position);
   virtual ~Camera() = default;

   void
   SetProjection(const glm::mat4&);

   [[nodiscard]] const glm::mat4&
   GetProjection() const;

   void
   SetView(const glm::mat4&);

   [[nodiscard]] const glm::mat4&
   GetView() const;

   void
   SetViewProjection(const glm::mat4&);

   [[nodiscard]] const glm::mat4&
   GetViewProjection() const;

   void
   SetPosition(const glm::vec3&);

   [[nodiscard]] const glm::vec3&
   GetPosition() const;

   [[nodiscard]] const glm::vec3&
   GetLookAtVec() const;

   [[nodiscard]] const glm::vec3&
   GetUpVec() const;

   [[nodiscard]] const glm::vec3&
   GetRightVec() const;

   virtual void
   MouseMovement(const glm::vec2& mouseMovement) = 0;

   virtual void
   MoveCamera(const glm::vec2& leftRightVec) = 0;

   virtual void
   RotateCamera(float angle, const glm::vec3& axis) = 0;

 protected:
   void
   UpdateViewMatrix();

   void
   UpdateViewProjection();

 protected:
   glm::mat4 projectionMat_ = glm::mat4(1.0f);
   glm::mat4 viewMat_ = glm::mat4(1.0f);
   glm::mat4 viewProjectionMat_ = glm::mat4(1.0f);

   glm::vec3 position_ = glm::vec3(0.0f);
   const glm::vec3 worldUp_ = glm::vec3(0.0f, -1.0f, 0.0f);
   glm::vec3 upVector_ = worldUp_;
   glm::vec3 rightVector_ = glm::vec3(0.0f, 0.0f, -1.0f);
   glm::vec3 lookAtDirection_ = glm::vec3(1.0f, 0.0f, 0.0f);
};
} // namespace shady::scene