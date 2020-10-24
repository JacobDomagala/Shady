#pragma once

#include "scene/camera.hpp"

namespace shady::scene {

class PerspectiveCamera : public Camera
{
 public:
   PerspectiveCamera(const glm::mat4& projection);
   PerspectiveCamera(float fieldOfView, float aspectRation, float near, float far);

   ~PerspectiveCamera() override = default;

   void
   MouseMovement(const glm::vec2& mouseMovement) override;

   void
   MoveCamera(const glm::vec2& leftRightVec) override;

 private:
   float m_yaw = -90.0f;
   float m_pitch = 0.0f;
   bool m_constrainPitch = true;

   const glm::vec3 m_worldUp = {0.0f, 1.0f, 0.0f};
};

} // namespace shady::scene