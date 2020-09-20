#pragma once

#include "camera.hpp"

namespace shady::render {

class OrthographicCamera : public Camera
{
 public:
   OrthographicCamera(const glm::mat4& projection);
   OrthographicCamera(float left, float right, float top, float bottom);

   ~OrthographicCamera() override = default;
};

} // namespace shady::render