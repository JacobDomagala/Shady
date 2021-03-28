#include "scene/orthographic_camera.hpp"

#include <glm/gtx/transform.hpp>

namespace shady::scene {

OrthographicCamera::OrthographicCamera(const glm::mat4& projection) : Camera(projection)
{
}

OrthographicCamera::OrthographicCamera(float left, float right, float top, float bottom)
{
   m_projectionMat = glm::ortho(left, right, top, bottom, -1.0f, 10.0f);
}

} // namespace shady::render