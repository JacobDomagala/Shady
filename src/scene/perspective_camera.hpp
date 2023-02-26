#pragma once

#include "scene/camera.hpp"

namespace shady::scene {

class PerspectiveCamera : public Camera
{
 public:
   PerspectiveCamera() = delete;
   PerspectiveCamera(const PerspectiveCamera&) = delete;
   PerspectiveCamera(PerspectiveCamera&&) = delete;
   PerspectiveCamera&
   operator=(const PerspectiveCamera&) = delete;
   PerspectiveCamera&
   operator=(PerspectiveCamera&&) = delete;

   PerspectiveCamera(const glm::mat4& projection, const glm::vec3& position);
   PerspectiveCamera(float fieldOfView, float aspectRatio, float nearClip, float farClip,
                     const glm::vec3& position);

   ~PerspectiveCamera() override = default;

   void
   MouseMovement(const glm::vec2& mouseMovement) override;

   void
   MoveCamera(const glm::vec2& leftRightVec) override;

   void
   RotateCamera(float angle, const glm::vec3& axis) override;

 private:
   float yaw_ = 0.0f;
   float pitch_ = 0.0f;
   bool constrainPitch_ = true;
};

} // namespace shady::scene
