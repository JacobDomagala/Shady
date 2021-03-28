#pragma once

#include "scene/camera.hpp"

namespace shady::scene {

class OrthographicCamera : public Camera
{
 public:
   explicit OrthographicCamera(const glm::mat4& projection);
   OrthographicCamera(float left, float right, float top, float bottom);

   ~OrthographicCamera() override = default;
};

} // namespace shady::render
