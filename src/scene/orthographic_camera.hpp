#pragma once

#include "scene/camera.hpp"

namespace shady::scene {

class OrthographicCamera : public Camera
{
 public:
   OrthographicCamera() = delete;
   OrthographicCamera(const OrthographicCamera&) = delete;
   OrthographicCamera(OrthographicCamera&&) = delete;
   OrthographicCamera&
   operator=(const OrthographicCamera&) = delete;
   OrthographicCamera&
   operator=(OrthographicCamera&&) = delete;


   OrthographicCamera(const glm::mat4& projection, const glm::vec3& position);
   OrthographicCamera(float left, float right, float top, float bottom, const glm::vec3& position);

   ~OrthographicCamera() override = default;
};

} // namespace shady::scene
