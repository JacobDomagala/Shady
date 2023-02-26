#include "scene/orthographic_camera.hpp"

#include <glm/gtx/transform.hpp>

namespace shady::scene {

OrthographicCamera::OrthographicCamera(const glm::mat4& projection, const glm::vec3& position)
   : Camera(projection, position)
{
}

OrthographicCamera::OrthographicCamera(float left, float right, float top, float bottom,
                                       const glm::vec3& position)
   //NOLINTNEXTLINE
   : Camera(glm::ortho(left, right, top, bottom, -1.0f, 10.0f), position)
{
}

} // namespace shady::scene