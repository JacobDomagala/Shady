#pragma once

#include "scene/camera.hpp"

namespace shady::scene {

class PerspectiveCamera : public Camera
{
 public:
   PerspectiveCamera(const glm::mat4& projection);
   PerspectiveCamera(float left, float right, float top, float bottom);

   ~PerspectiveCamera() override = default;
};

} // namespace shady::render